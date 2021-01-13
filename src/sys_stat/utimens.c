/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <sys/stat.h>
#include <fcntl.h>
#include <misc.h>
#include <errno.h>
#include <wlibc_errors.h>
#include <Windows.h>
#include <time.h>
#include <fcntl_internal.h>

FILETIME get_current_filetime()
{
	SYSTEMTIME systemtime;
	FILETIME filetime;
	GetSystemTime(&systemtime);
	SystemTimeToFileTime(&systemtime, &filetime);
	return filetime;
}

LARGE_INTEGER timespec_to_FILETIME(const struct timespec time)
{
	LARGE_INTEGER L;
	L.QuadPart = time.tv_sec * 10000000 + time.tv_nsec / 100;
	L.QuadPart += 116444736000000000LL;
	return L;
}

int common_utimens(HANDLE file, const struct timespec times[2])
{
	FILE_BASIC_INFO BASIC_INFO;
	if (!GetFileInformationByHandleEx(file, FileBasicInfo, &BASIC_INFO, sizeof(FILE_BASIC_INFO)))
	{
		map_win32_error_to_wlibc(GetLastError());
		return -1;
	}

	FILETIME current_filetime = get_current_filetime();
	if (times == NULL)
	{
		BASIC_INFO.LastAccessTime.HighPart = current_filetime.dwHighDateTime;
		BASIC_INFO.LastAccessTime.LowPart = current_filetime.dwLowDateTime;
		BASIC_INFO.LastWriteTime.HighPart = current_filetime.dwHighDateTime;
		BASIC_INFO.LastWriteTime.LowPart = current_filetime.dwLowDateTime;
	}

	else
	{
		if (times[0].tv_nsec == UTIME_NOW)
		{
			BASIC_INFO.LastAccessTime.HighPart = current_filetime.dwHighDateTime;
			BASIC_INFO.LastAccessTime.LowPart = current_filetime.dwLowDateTime;
		}
		else if (times[0].tv_nsec != UTIME_OMIT)
		{
			BASIC_INFO.LastAccessTime.QuadPart = timespec_to_FILETIME(times[0]).QuadPart;
		}

		if (times[1].tv_nsec == UTIME_NOW)
		{
			BASIC_INFO.LastWriteTime.HighPart = current_filetime.dwHighDateTime;
			BASIC_INFO.LastWriteTime.LowPart = current_filetime.dwLowDateTime;
		}
		else if (times[1].tv_nsec != UTIME_OMIT)
		{
			BASIC_INFO.LastWriteTime.QuadPart = timespec_to_FILETIME(times[0]).QuadPart;
		}
	}

	if (!SetFileInformationByHandle(file, FileBasicInfo, &BASIC_INFO, sizeof(FILE_BASIC_INFO)))
	{
		map_win32_error_to_wlibc(GetLastError());
		return -1;
	}

	return 0;
}

int common_utimensat(int dirfd, const wchar_t *wname, const struct timespec times[2], int flags)
{
	wchar_t *newname = NULL;
	int symfile_flags = 0;
	int use_dirfd = 1;

	if (flags != 0 && flags != AT_SYMLINK_NOFOLLOW)
	{
		errno = EINVAL;
		return -1;
	}

	int derefernce_symlinks = 1;
	if (flags == AT_SYMLINK_NOFOLLOW)
	{
		derefernce_symlinks = 0;
	}

	if (derefernce_symlinks == 0)
	{
		symfile_flags = FILE_FLAG_OPEN_REPARSE_POINT;
	}

	if (dirfd == AT_FDCWD || is_absolute_pathw(wname))
	{
		use_dirfd = 0;
		newname = (wchar_t *)wname;
	}
	else if (!validate_dirfd(dirfd))
	{
		return -1;
	}

	if (use_dirfd)
	{
		const wchar_t *dirpath = get_fd_path(dirfd);
		newname = wcstrcat(dirpath, wname);
	}

	HANDLE file = CreateFile(newname, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
							 FILE_FLAG_BACKUP_SEMANTICS | symfile_flags, NULL);
	if (file == INVALID_HANDLE_VALUE)
	{
		map_win32_error_to_wlibc(GetLastError());
		free(newname);
		return -1;
	}

	int status = common_utimens(file, times);

	if (use_dirfd)
	{
		free(newname);
	}

	CloseHandle(file);
	return status;
}

int wlibc_utimensat(int dirfd, const char *name, const struct timespec times[2], int flags)
{
	if (name == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	wchar_t *wname = mb_to_wc(name);
	int status = common_utimensat(dirfd, wname, times, flags);
	free(wname);

	return status;
}

int wlibc_wutimensat(int dirfd, const wchar_t *wname, const struct timespec times[2], int flags)
{
	if (wname == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	return common_utimensat(dirfd, wname, times, flags);
}

int wlibc_futimens(int fd, const struct timespec times[2])
{
	if (!validate_active_fd(fd))
	{
		return -1;
	}

	HANDLE file = get_fd_handle(fd);
	return common_utimens(file, times);
}
