/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <unistd.h>
#include <internal/misc.h>
#include <Windows.h>
#include <internal/error.h>
#include <internal/fcntl.h>

int common_link(int olddirfd, const char *restrict source, int newdirfd, const char *restrict target, int flags)
{
	HANDLE handle = INVALID_HANDLE_VALUE;
	if (flags != AT_EMPTY_PATH)
	{

		wchar_t *u16_ntsource = get_absolute_ntpath(olddirfd, source);
		if (u16_ntsource == NULL)
		{
			errno = ENOENT;
			return -1;
		}

		ULONG options = FILE_NON_DIRECTORY_FILE; // We can't create hardlinks between directories
		if (flags != AT_SYMLINK_FOLLOW)
		{
			options |= FILE_OPEN_REPARSE_POINT;
		}

		handle = just_open(u16_ntsource, FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES | SYNCHRONIZE, 0, FILE_OPEN, options);
		free(u16_ntsource);

		if (handle == INVALID_HANDLE_VALUE)
		{
			// errno wil be set by just_open
			return -1;
		}
	}
	else
	{
		handle = get_fd_handle(olddirfd);
	}

	wchar_t *u16_nttarget = get_absolute_ntpath(newdirfd, target);
	if (u16_nttarget == NULL)
	{
		errno = ENOENT; // bad path
		return -1;
	}

	int length = wcslen(u16_nttarget) * sizeof(wchar_t);
	size_t size_of_link_info = sizeof(FILE_LINK_INFORMATION) - sizeof(WCHAR) + length;
	IO_STATUS_BLOCK I;
	PFILE_LINK_INFORMATION link_info = malloc(size_of_link_info);
	memset(link_info, 0, size_of_link_info);
	// No need to set RootDirectory as we are zeroing the memory
	link_info->Flags = FILE_LINK_POSIX_SEMANTICS;
	link_info->FileNameLength = length;
	memcpy(link_info->FileName, u16_nttarget, length);
	free(u16_nttarget);

	NTSTATUS status = NtSetInformationFile(handle, &I, link_info, size_of_link_info, FileLinkInformationEx);
	free(link_info);
	if (flags != AT_EMPTY_PATH)
	{
		NtClose(handle);
	}

	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	return 0;
}

int wlibc_common_link(int olddirfd, const char *restrict source, int newdirfd, const char *restrict target, int flags)
{
	if (target == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	if (flags != AT_EMPTY_PATH)
	{
		if (source == NULL)
		{
			errno = ENOENT;
			return -1;
		}

		if (olddirfd != AT_FDCWD && get_fd_type(olddirfd) != DIRECTORY_HANDLE)
		{
			errno = ENOTDIR;
			return -1;
		}
	}
	else
	{
		if (get_fd_type(olddirfd) != FILE_HANDLE)
		{
			errno = EBADF;
			return -1;
		}

		if (get_fd_flags(olddirfd) & O_EXCL)
		{
			errno = EBADF;
			return -1;
		}
	}

	if (newdirfd != AT_FDCWD && get_fd_type(newdirfd) != DIRECTORY_HANDLE)
	{
		errno = ENOTDIR;
		return -1;
	}

	if (flags != 0 && flags != AT_EMPTY_PATH && flags != AT_SYMLINK_FOLLOW)
	{
		errno = EINVAL;
		return -1;
	}

	return common_link(olddirfd, source, newdirfd, target, flags);
}
