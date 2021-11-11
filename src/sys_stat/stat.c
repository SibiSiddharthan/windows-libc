/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <sys/stat.h>
#include <Windows.h>
#include <internal/error.h>
#include <internal/misc.h>
#include <errno.h>
#include <internal/fcntl.h>
#include <time.h>
#include <unistd.h>

/* 116444736000000000 is the number of 100 nanosecond intervals from
   January 1st 1601 to January 1st 1970 (UTC)
*/
struct timespec LARGE_INTEGER_to_timespec(LARGE_INTEGER LT)
{
	struct timespec result;
	time_t epoch = LT.QuadPart - 116444736000000000LL;
	result.tv_sec = epoch / 10000000;
	result.tv_nsec = (epoch % 10000000) * 100;
	return result;
}

int do_stat(HANDLE handle, struct stat *restrict statbuf)
{
	IO_STATUS_BLOCK I;
	NTSTATUS status;

	FILE_FS_DEVICE_INFORMATION device_info;
	status = NtQueryVolumeInformationFile(handle, &I, &device_info, sizeof(FILE_FS_DEVICE_INFORMATION), FileFsDeviceInformation);
	// This is important
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	memset(statbuf, 0, sizeof(struct stat));

	DEVICE_TYPE type = device_info.DeviceType;
	if (type == FILE_DEVICE_DISK)
	{
		FILE_STAT_INFORMATION stat_info;
		status = NtQueryInformationFile(handle, &I, &stat_info, sizeof(FILE_STAT_INFORMATION), FileStatInformation);
		if (status != STATUS_SUCCESS)
		{
			map_ntstatus_to_errno(status);
			return -1;
		}

		DWORD attributes = stat_info.FileAttributes;
		// From readdir.c
		if (attributes & FILE_ATTRIBUTE_REPARSE_POINT)
		{
			ULONG reparse_tag = stat_info.ReparseTag;
			if (reparse_tag == IO_REPARSE_TAG_SYMLINK || reparse_tag == IO_REPARSE_TAG_MOUNT_POINT)
			{
				statbuf->st_mode = S_IFLNK;
				statbuf->st_mode |= S_IREAD | S_IWRITE | S_IEXEC;
			}
			// if(reparse_tag == IO_REPARSE_TAG_AF_UNIX)
			//{
			//	statbuf->st_mode = S_IFSOCK;
			//	statbuf->st_mode |= S_IREAD | S_IWRITE | S_IEXEC;
			//}
		}
		else if (attributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			statbuf->st_mode = S_IFDIR;
			statbuf->st_mode |= S_IREAD | S_IWRITE | S_IEXEC;
		}
		else if ((attributes & ~(FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_ARCHIVE |
								 FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_TEMPORARY | FILE_ATTRIBUTE_SPARSE_FILE | FILE_ATTRIBUTE_COMPRESSED |
								 FILE_ATTRIBUTE_NOT_CONTENT_INDEXED | FILE_ATTRIBUTE_ENCRYPTED)) == 0)
		{
			statbuf->st_mode = S_IFREG;
			if (attributes & FILE_ATTRIBUTE_READONLY)
			{
				statbuf->st_mode |= S_IREAD;
			}
			else
			{
				statbuf->st_mode |= S_IREAD | S_IWRITE;
			}
		}

		//// Set the S_IEXEC bit here as it requires the name
		// if (has_executable_extenstion(wbuf + 4))
		//{
		//	statbuf->st_mode |= S_IEXEC;
		//}

		statbuf->st_ino = stat_info.FileId.QuadPart;
		statbuf->st_nlink = stat_info.NumberOfLinks;
		statbuf->st_size = stat_info.EndOfFile.QuadPart;

		// st_[amc]tim
		statbuf->st_atim = LARGE_INTEGER_to_timespec(stat_info.LastAccessTime);
		statbuf->st_mtim = LARGE_INTEGER_to_timespec(stat_info.LastWriteTime);
		statbuf->st_ctim = LARGE_INTEGER_to_timespec(stat_info.CreationTime);

		if (statbuf->st_mode & S_IFLNK)
		{
			statbuf->st_size = -1;
			PREPARSE_DATA_BUFFER reparse_buffer = (PREPARSE_DATA_BUFFER)malloc(MAXIMUM_REPARSE_DATA_BUFFER_SIZE);
			status = NtFsControlFile(handle, NULL, NULL, NULL, &I, FSCTL_GET_REPARSE_POINT, NULL, 0, reparse_buffer,
									 MAXIMUM_REPARSE_DATA_BUFFER_SIZE);
			if (status != STATUS_SUCCESS)
			{
				// don't return just set errno
				map_ntstatus_to_errno(status);
			}

			if (reparse_buffer->ReparseTag == IO_REPARSE_TAG_SYMLINK)
			{
				if (reparse_buffer->SymbolicLinkReparseBuffer.PrintNameLength != 0)
				{
					statbuf->st_size = reparse_buffer->SymbolicLinkReparseBuffer.PrintNameLength / sizeof(WCHAR);
				}
				else if (reparse_buffer->SymbolicLinkReparseBuffer.SubstituteNameLength != 0)
				{
					statbuf->st_size = reparse_buffer->SymbolicLinkReparseBuffer.SubstituteNameLength / sizeof(WCHAR);
				}
			}

			if (reparse_buffer->ReparseTag == IO_REPARSE_TAG_MOUNT_POINT)
			{
				if (reparse_buffer->MountPointReparseBuffer.PrintNameLength != 0)
				{
					statbuf->st_size = reparse_buffer->MountPointReparseBuffer.PrintNameLength / sizeof(WCHAR);
				}
				else if (reparse_buffer->MountPointReparseBuffer.SubstituteNameLength != 0)
				{
					statbuf->st_size = reparse_buffer->MountPointReparseBuffer.SubstituteNameLength / sizeof(WCHAR);
				}
			}
			free(reparse_buffer);
		}

		FILE_FS_SIZE_INFORMATION size_info;
		status = NtQueryVolumeInformationFile(handle, &I, &size_info, sizeof(FILE_FS_SIZE_INFORMATION), FileFsSizeInformation);
		if (status == STATUS_SUCCESS)
		{
			statbuf->st_blksize = size_info.BytesPerSector * size_info.SectorsPerAllocationUnit;
			if (statbuf->st_mode & S_IFDIR)
			{
				statbuf->st_size = statbuf->st_blksize;
			}
			statbuf->st_blocks = statbuf->st_size / statbuf->st_blksize + (statbuf->st_size % statbuf->st_blksize == 0 ? 0 : 1);
		}
		else
		{
			// just set errno
			map_ntstatus_to_errno(status);
		}

		PFILE_FS_VOLUME_INFORMATION volume_info = (PFILE_FS_VOLUME_INFORMATION)malloc(128); // Max label length is 32(WCHAR) or 64 bytes
		status = NtQueryVolumeInformationFile(handle, &I, volume_info, 1024, FileFsVolumeInformation);
		if (status == STATUS_SUCCESS)
		{
			statbuf->st_dev = volume_info->VolumeSerialNumber;
		}
		else
		{
			// just set errno
			map_ntstatus_to_errno(status);
		}

		free(volume_info);
	}
	else if (type == FILE_DEVICE_NULL || type == FILE_DEVICE_CONSOLE)
	{
		statbuf->st_mode = S_IFCHR;
		statbuf->st_nlink = 1;
		// To differentiate between NUL and CON use st_dev and st_rdev.
		// We don't use st_ino because they have no meaning as these files are devices in the object manager.
		if (type == FILE_DEVICE_NULL)
		{
			statbuf->st_rdev = 1;
			statbuf->st_dev = 1;
		}
		else // (type == FILE_DEVICE_CONSOLE)
		{
			statbuf->st_rdev = 2;
			statbuf->st_dev = 2;
		}
	}
	else if (type == FILE_DEVICE_NAMED_PIPE)
	{
		statbuf->st_mode = S_IFIFO;
		statbuf->st_rdev = 0;
		statbuf->st_nlink = 1;
		statbuf->st_dev = 3;
	}

	return 0;
}

int common_stat(int dirfd, const char *restrict name, struct stat *restrict statbuf, int flags)
{
	wchar_t *u16_ntpath = get_absolute_ntpath(dirfd, name);
	if (u16_ntpath == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	HANDLE handle = just_open(u16_ntpath, FILE_READ_ATTRIBUTES | READ_CONTROL, 0, FILE_OPEN,
							  flags == AT_SYMLINK_NOFOLLOW ? FILE_OPEN_REPARSE_POINT : 0);
	if (handle == INVALID_HANDLE_VALUE)
	{
		// errno will be set by just_open
		return -1;
	}

	int result = do_stat(handle, statbuf);
	NtClose(handle);
	return result;
}

int wlibc_common_stat(int dirfd, const char *restrict name, struct stat *restrict statbuf, int flags)
{
	if (statbuf == NULL)
	{
		errno = EINVAL;
		return -1;
	}

	if (flags != 0 && flags != AT_SYMLINK_NOFOLLOW && flags != AT_EMPTY_PATH)
	{
		errno = EINVAL;
		return -1;
	}

	if (flags != AT_EMPTY_PATH)
	{
		VALIDATE_PATH_AND_DIRFD(name, dirfd);
	}
	else
	{
		if (!validate_fd(dirfd))
		{
			errno = EBADF;
			return -1;
		}

		return do_stat(get_fd_handle(dirfd), statbuf);
	}

	return common_stat(dirfd, name, statbuf, flags);
}
