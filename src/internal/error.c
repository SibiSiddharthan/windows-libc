/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/error.h>
#include <errno.h>
#include <Windows.h>

void map_doserror_to_errno(DWORD error)
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
	case ERROR_BROKEN_PIPE:
	case ERROR_BAD_PIPE:
	case ERROR_PIPE_BUSY:
	case ERROR_NO_DATA:
	case ERROR_PIPE_NOT_CONNECTED:
		errno = EPIPE;
	default:
		break;
	};
}

void map_ntstatus_to_errno(NTSTATUS error)
{
	switch (error)
	{
	case STATUS_NO_MEMORY:
	case STATUS_FATAL_MEMORY_EXHAUSTION:
		errno = ENOMEM;
		break;
	case STATUS_ACCESS_DENIED:
	case STATUS_SHARING_VIOLATION:
	case STATUS_SECTION_PROTECTION:
		errno = EACCES;
		break;
	case STATUS_CANNOT_DELETE:
	case STATUS_MUTANT_NOT_OWNED:
		errno = EPERM;
		break;
	case STATUS_OBJECT_NAME_NOT_FOUND:
	case STATUS_OBJECT_PATH_NOT_FOUND:
		errno = ENOENT;
		break;
	case STATUS_TOO_MANY_OPENED_FILES:
		errno = EMFILE;
		break;
	case STATUS_NAME_TOO_LONG:
		errno = ENAMETOOLONG;
	case STATUS_NOT_A_DIRECTORY:
		errno = ENOTDIR;
		break;
	case STATUS_DIRECTORY_NOT_EMPTY:
		errno = ENOTEMPTY;
		break;
	case STATUS_OBJECT_NAME_EXISTS:
	case STATUS_OBJECT_NAME_COLLISION:
		errno = EEXIST;
		break;
	case STATUS_INVALID_HANDLE:
		errno = EBADF;
		break;
	case STATUS_IO_TIMEOUT:
	case STATUS_IO_UNALIGNED_WRITE:
	case STATUS_FILE_LOCK_CONFLICT:
		errno = EIO;
		break;
	case STATUS_INVALID_PARAMETER:
	case STATUS_ILLEGAL_FUNCTION:
	case STATUS_OBJECT_NAME_INVALID:
	case STATUS_INVALID_EA_NAME:
	case STATUS_EA_LIST_INCONSISTENT:
	case STATUS_SECTION_TOO_BIG:
	case STATUS_MAPPED_ALIGNMENT:
		errno = EINVAL;
		break;
	case STATUS_FILE_IS_A_DIRECTORY:
		errno = EISDIR;
		break;
	case STATUS_PIPE_BROKEN:
	case STATUS_PIPE_BUSY:
	case STATUS_PIPE_DISCONNECTED:
	case STATUS_PIPE_NOT_AVAILABLE:
		errno = EPIPE;
		break;
	case STATUS_INVALID_DEVICE_REQUEST:
		errno = EROFS;
		break;
	case STATUS_NOT_SUPPORTED:
	case STATUS_EAS_NOT_SUPPORTED:
		errno = ENOTSUP;
		break;
	case STATUS_INSUFFICIENT_RESOURCES:
	case STATUS_LOCK_NOT_GRANTED:
	case STATUS_RANGE_NOT_LOCKED: // CHECK
		errno = ENOLCK;
		break;
	case STATUS_BUFFER_TOO_SMALL:
		errno = ERANGE;
		break;
	case STATUS_EA_TOO_LARGE:
		errno = E2BIG;
		break;
	case STATUS_TIMEOUT:
		errno = ETIMEDOUT;
		break;
	default:
		break;
	};
}
