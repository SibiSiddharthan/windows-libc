/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_SYS_FILE_H
#define WLIBC_SYS_FILE_H

#include <wlibc-macros.h>

_WLIBC_BEGIN_DECLS

#define LOCK_SH 1 // Shared lock
#define LOCK_EX 2 // Exclusive lock
#define LOCK_UN 4 // Unlock
#define LOCK_NB 8 // Don't block when locking

WLIBC_API int wlibc_flock(int fd, int operation);

WLIBC_INLINE int flock(int fd, int operation)
{
	return wlibc_flock(fd, operation);
}

_WLIBC_END_DECLS

#endif
