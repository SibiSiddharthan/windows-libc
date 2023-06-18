/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_SYS_IOCTL_H
#define WLIBC_SYS_IOCTL_H

#include <wlibc.h>
#include <stdarg.h>

_WLIBC_BEGIN_DECLS

WLIBC_API int wlibc_ioctl(int fd, unsigned long request, va_list args);

WLIBC_INLINE int ioctl(int fd, unsigned long request, ...)
{
	va_list args;
	va_start(args, request);
	int result = wlibc_ioctl(fd, request, args);
	va_end(args);
	return result;
}

_WLIBC_END_DECLS

#endif
