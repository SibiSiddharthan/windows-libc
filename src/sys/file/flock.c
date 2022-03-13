/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <internal/fcntl.h>
#include <errno.h>
#include <sys/file.h>

int wlibc_flock(int fd, int operation)
{
	NTSTATUS status;
	IO_STATUS_BLOCK io;
	HANDLE handle;
	LARGE_INTEGER offset, length;
	fdinfo info;

	get_fdinfo(fd, &info);

	// Only support regular files
	if (info.type != FILE_HANDLE)
	{
		errno = EBADF;
		return -1;
	}

	int nonblock = (operation & LOCK_NB) ? 1 : 0;
	operation = operation & ~LOCK_NB;

	if (operation != LOCK_SH && operation != LOCK_EX && operation != LOCK_UN)
	{
		errno = EINVAL;
		return -1;
	}

	handle = info.handle;

	// We are always locking the entire file
	offset.QuadPart = 0;
	length.QuadPart = -1;

	switch (operation)
	{
	case LOCK_SH:
		status = NtLockFile(handle, NULL, NULL, NULL, &io, &offset, &length, 0, nonblock == 1 ? TRUE : FALSE, FALSE);
		break;
	case LOCK_EX:
		status = NtLockFile(handle, NULL, NULL, NULL, &io, &offset, &length, 0, nonblock == 1 ? TRUE : FALSE, TRUE);
		break;
	case LOCK_UN:
		status = NtUnlockFile(handle, &io, &offset, &length, 0);
		break;
	default:
		// This should never happen.
		status = STATUS_INVALID_PARAMETER_2;
		break;
	}

	if (status != STATUS_SUCCESS)
	{
		if (status == STATUS_LOCK_NOT_GRANTED && nonblock == 1)
		{
			errno = EWOULDBLOCK;
		}
		else
		{
			map_ntstatus_to_errno(status);
		}
		return -1;
	}

	return 0;
}
