/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <unistd.h>
#include <Windows.h>
#include <wlibc_errors.h>
#include <errno.h>
#include <fcntl_internal.h>

off_t wlibc_lseek(int fd, off_t offset, int whence)
{
	if (!validate_active_ffd(fd))
	{
		return -1;
	}

	if (whence > 2 || whence < 0)
	{
		errno = EINVAL;
		return -1;
	}

	HANDLE file = get_fd_handle(fd);
	// Fail if we are not a disk file
	if (GetFileType(file) != FILE_TYPE_DISK)
	{
		errno = ESPIPE;
		return -1;
	}

	LARGE_INTEGER _offset, _newpos;
	_offset.QuadPart = offset;
	if (!SetFilePointerEx(file, _offset, &_newpos, whence))
	{
		map_win32_error_to_wlibc(GetLastError());
		return -1;
	}

	return _newpos.QuadPart;
}
