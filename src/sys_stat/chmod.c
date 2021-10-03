/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <sys/stat.h>
#include <internal/misc.h>
#include <errno.h>
#include <internal/error.h>
#include <Windows.h>
#include <internal/fcntl.h>

int common_chmod(const wchar_t *wname, mode_t mode)
{
	DWORD attributes = GetFileAttributes(wname);
	if (attributes == INVALID_FILE_ATTRIBUTES)
	{
		errno = ENOENT;
		return -1;
	}

	// If we don't have write permission, make the file READONLY. This is what MSVC does
	// Maybe later we can use acls for this
	if (mode & S_IWRITE)
	{
		attributes &= ~FILE_ATTRIBUTE_READONLY;
	}
	else
	{
		attributes |= FILE_ATTRIBUTE_READONLY;
	}

	if (!SetFileAttributes(wname, attributes))
	{
		map_win32_error_to_wlibc(GetLastError());
		return -1;
	}

	return 0;
}

int wlibc_chmod(const char *name, mode_t mode)
{
	VALIDATE_PATH(name, ENOENT);

	wchar_t *wname = mb_to_wc(name);
	int status = common_chmod(wname, mode);
	free(wname);

	return status;
}

int wlibc_wchmod(const wchar_t *wname, mode_t mode)
{
	VALIDATE_PATH(wname, ENOENT);

	return common_chmod(wname, mode);
}

int wlibc_fchmod(int fd, mode_t mode)
{

	if (!validate_fd(fd))
	{
		errno = EBADF;
		return -1;
	}

	const wchar_t *wname = get_fd_path(fd);
	return common_chmod(wname, mode);
}
