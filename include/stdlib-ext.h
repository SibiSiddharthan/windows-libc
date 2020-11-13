/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_STDLIB_EXT_H
#define WLIBC_STDLIB_EXT_H

#include <wlibc-macros.h>
#include <wchar.h>

_WLIBC_BEGIN_DECLS

WLIBC_API int wlibc_setenv(const char *name, const char *value, int overwrite);
WLIBC_API int wlibc_wsetenv(const wchar_t *name, const wchar_t *value, int overwrite);

WLIBC_INLINE int setenv(const char *name, const char *value, int overwrite)
{
	return wlibc_setenv(name, value, overwrite);
}

WLIBC_INLINE int wsetenv(const wchar_t *name, const wchar_t *value, int overwrite)
{
	return wlibc_wsetenv(name, value, overwrite);
}

WLIBC_API int wlibc_unsetenv(const char *name);
WLIBC_API int wlibc_wunsetenv(const wchar_t *name);

WLIBC_INLINE int unsetenv(const char *name)
{
	return wlibc_unsetenv(name);
}

WLIBC_INLINE int wunsetenv(const wchar_t *name)
{
	return wlibc_wunsetenv(name);
}

_WLIBC_END_DECLS

#endif
