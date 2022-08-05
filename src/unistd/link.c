/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <internal/fcntl.h>
#include <internal/path.h>
#include <unistd.h>

int do_link(HANDLE handle, int dirfd, const char *restrict target)
{
	NTSTATUS status;
	IO_STATUS_BLOCK io;
	UNICODE_STRING *u16_nttarget = NULL;
	PFILE_LINK_INFORMATION link_info = NULL;
	ULONG size_of_link_info;

	u16_nttarget = get_absolute_ntpath(dirfd, target);
	if (u16_nttarget == NULL)
	{
		// Bad path
		// errno will be set by 'get_absolute_ntpath'.
		return -1;
	}

	size_of_link_info = sizeof(FILE_LINK_INFORMATION) - sizeof(WCHAR) + u16_nttarget->Length;
	link_info = (PFILE_LINK_INFORMATION)RtlAllocateHeap(NtCurrentProcessHeap(), HEAP_ZERO_MEMORY, size_of_link_info);

	if (link_info == NULL)
	{
		errno = ENOMEM;
		return -1;
	}

	// No need to set RootDirectory as we are zeroing the memory
	link_info->Flags = FILE_LINK_POSIX_SEMANTICS;
	link_info->FileNameLength = u16_nttarget->Length;
	memcpy(link_info->FileName, u16_nttarget->Buffer, u16_nttarget->Length);

	status = NtSetInformationFile(handle, &io, link_info, size_of_link_info, FileLinkInformationEx);

	RtlFreeHeap(NtCurrentProcessHeap(), 0, u16_nttarget);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, link_info);

	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	return 0;
}

int common_link(int olddirfd, const char *restrict source, int newdirfd, const char *restrict target, int flags)
{
	HANDLE handle = INVALID_HANDLE_VALUE;
	ULONG options = FILE_NON_DIRECTORY_FILE; // We can't create hardlinks between directories
	if (flags != AT_SYMLINK_FOLLOW)
	{
		options |= FILE_OPEN_REPARSE_POINT;
	}

	// If the file does not have FILE_WRITE_ATTRIBUTES, link creation fails.
	handle = just_open(olddirfd, source, FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES | SYNCHRONIZE, options);
	if (handle == INVALID_HANDLE_VALUE)
	{
		// errno wil be set by just_open
		return -1;
	}

	int result = do_link(handle, newdirfd, target);
	NtClose(handle);

	return result;
}

int wlibc_common_link(int olddirfd, const char *restrict source, int newdirfd, const char *restrict target, int flags)
{
	if (flags != 0 && flags != AT_EMPTY_PATH && flags != AT_SYMLINK_FOLLOW)
	{
		errno = EINVAL;
		return -1;
	}

	VALIDATE_PATH_AND_DIRFD(target, newdirfd);

	if (flags != AT_EMPTY_PATH)
	{
		VALIDATE_PATH_AND_DIRFD(source, olddirfd);
		return common_link(olddirfd, source, newdirfd, target, flags);
	}
	else
	{
		if (get_fd_type(olddirfd) != FILE_HANDLE || get_fd_flags(olddirfd) & O_EXCL)
		{
			errno = EBADF;
			return -1;
		}

		HANDLE handle = get_fd_handle(olddirfd);
		return do_link(handle, newdirfd, target);
	}
}
