/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_IO_H
#define WLIBC_IO_H

#include <wlibc.h>

_WLIBC_BEGIN_DECLS

/*
 * Below functions are no-op and defined here for compatibility. We always do binary IO with files.
 * By introducing our io.h we prevent including MSVC's io.h which redeclares a few symbols (rename, remove).
 */

WLIBC_INLINE int _setmode(int fd, int mode)
{
	return 0;
}

#define setmode _setmode

// From fcntl.h
WLIBC_API intptr_t wlibc_get_osfhandle(int fd);
WLIBC_API int wlibc_open_osfhandle(intptr_t handle, int flags);

WLIBC_INLINE intptr_t _get_osfhandle(int fd)
{
	return wlibc_get_osfhandle(fd);
}

WLIBC_INLINE int _open_osfhandle(intptr_t handle, int flags)
{
	return wlibc_open_osfhandle(handle, flags);
}

_WLIBC_END_DECLS

#endif
