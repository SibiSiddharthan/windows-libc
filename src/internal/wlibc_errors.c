/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <wlibc_errors.h>
#include <errno.h>
#include <windows.h>

void map_win32_error_to_wlibc(unsigned long error)
{
	switch (error)
	{
	case ERROR_NOT_ENOUGH_MEMORY:
	case ERROR_OUTOFMEMORY:
		errno = ENOMEM;
		break;
	case ERROR_ACCESS_DENIED:
	case ERROR_SHARING_VIOLATION:
		errno = EACCES;
		break;
	case ERROR_FILE_NOT_FOUND:
	case ERROR_PATH_NOT_FOUND:
		errno = ENOENT;
		break;
	case ERROR_TOO_MANY_OPEN_FILES:
		errno = EMFILE;
		break;
	case ERROR_DIRECTORY:
		errno = ENOTDIR;
		break;
	case ERROR_DIR_NOT_EMPTY:
		errno = ENOTEMPTY;
		break;
	case ERROR_ALREADY_EXISTS:
	case ERROR_FILE_EXISTS:
		errno = EEXIST;
		break;
	case ERROR_INVALID_HANDLE:
		errno = EBADF;
		break;
	case ERROR_WRITE_FAULT:
	case ERROR_READ_FAULT:
		errno = EIO;
		break;
	case ERROR_INVALID_PARAMETER:
	case ERROR_INVALID_FUNCTION:
	case ERROR_INVALID_REPARSE_DATA: // We map this to EINVAL instead of ENOENT (glibc does this)
	case ERROR_INVALID_NAME:
		errno = EINVAL;
		break;
	default:
		break;
	};
}
