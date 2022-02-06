/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_ERR_H
#define WLIBC_ERR_H

#include <wlibc.h>
#include <stdarg.h>
#include <errno.h>

_WLIBC_BEGIN_DECLS

WLIBC_API void wlibc_error(int status, int errnum, const char *filename, unsigned int linenum, int do_strerror, const char *format, va_list args);
WLIBC_API void wlibc_warn(int errnum, const char *filename, unsigned int linenum, int do_strerror, const char *format, va_list args);

WLIBC_INLINE void err(int status, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	wlibc_error(status, errno, NULL, 0, 1, format, args);
	va_end(args);
}

WLIBC_INLINE void errx(int status, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	wlibc_error(status, errno, NULL, 0, 0, format, args);
	va_end(args);
}

WLIBC_INLINE void verr(int status, const char *format, va_list args)
{
	wlibc_error(status, errno, NULL, 0, 1, format, args);
}

WLIBC_INLINE void verrx(int status, const char *format, va_list args)
{
	wlibc_error(status, errno, NULL, 0, 0, format, args);
}

WLIBC_INLINE void warn(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	wlibc_warn(errno, NULL, 0, 1, format, args);
	va_end(args);
}

WLIBC_INLINE void warnx(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	wlibc_warn(errno, NULL, 0, 0, format, args);
	va_end(args);
}

WLIBC_INLINE void vwarn(const char *format, va_list args)
{
	wlibc_warn(errno, NULL, 0, 1, format, args);
}

WLIBC_INLINE void vwarnx(const char *format, va_list args)
{
	wlibc_warn(errno, NULL, 0, 0, format, args);
}

_WLIBC_END_DECLS

#endif
