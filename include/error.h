/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_ERROR_H
#define WLIBC_ERROR_H

#include <wlibc.h>
#include <stdarg.h>

_WLIBC_BEGIN_DECLS

extern unsigned int error_message_count;

WLIBC_API void wlibc_error(int status, int errnum, const char *filename, unsigned int linenum, int do_strerror, const char *format, va_list args);
WLIBC_API void wlibc_warn(int errnum, const char *filename, unsigned int linenum, int do_strerror, const char *format, va_list args);

WLIBC_INLINE void error(int status, int errnum, const char *format, ...)
{
	va_list args;
	va_start(args, format);

	if (status)
	{
		wlibc_error(status, errnum, NULL, 0, 1, format, args);
	}
	else
	{
		wlibc_warn(errnum, NULL, 0, 1, format, args);
	}

	va_end(args);
}

WLIBC_INLINE void verror(int status, int errnum, const char *format, va_list args)
{
	if (status)
	{
		wlibc_error(status, errnum, NULL, 0, 1, format, args);
	}
	else
	{
		wlibc_warn(errnum, NULL, 0, 1, format, args);
	}
}

WLIBC_INLINE void error_at_line(int status, int errnum, const char *filename, unsigned int linenum, const char *format, ...)
{
	va_list args;
	va_start(args, format);

	if (status)
	{
		wlibc_error(status, errnum, filename, linenum, 1, format, args);
	}
	else
	{
		wlibc_warn(errnum, filename, linenum, 1, format, args);
	}

	va_end(args);
}

WLIBC_INLINE void verror_at_line(int status, int errnum, const char *filename, unsigned int linenum, const char *format, va_list args)
{
	if (status)
	{
		wlibc_error(status, errnum, filename, linenum, 1, format, args);
	}
	else
	{
		wlibc_warn(errnum, filename, linenum, 1, format, args);
	}
}

_WLIBC_END_DECLS

#endif
