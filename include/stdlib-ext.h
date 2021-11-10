/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_STDLIB_EXT_H
#define WLIBC_STDLIB_EXT_H

#include <wlibc-macros.h>
#include <wchar.h>

_WLIBC_BEGIN_DECLS

WLIBC_API int wlibc_setenv(const char *restrict name, const char *restrict value, int overwrite);
WLIBC_API int wlibc_wsetenv(const wchar_t *restrict name, const wchar_t *restrict value, int overwrite);

WLIBC_INLINE int setenv(const char *restrict name, const char *restrict value, int overwrite)
{
	return wlibc_setenv(name, value, overwrite);
}

WLIBC_INLINE int wsetenv(const wchar_t *restrict name, const wchar_t *restrict value, int overwrite)
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

/*
#define CREATE_FILE 1
#define CREATE_DIR  2
#define TRY_NAME    3
*/

WLIBC_API int wlibc_common_mkstemp(char *template, int suffixlen, int flags, int operation);

WLIBC_INLINE int mkstemp(char *template)
{
	return wlibc_common_mkstemp(template, 0, 0, 1);
}

WLIBC_INLINE int mkostemp(char *template, int flags)
{
	return wlibc_common_mkstemp(template, 0, flags, 1);
}

WLIBC_INLINE int mkstemps(char *template, int suffixlen)
{
	return wlibc_common_mkstemp(template, suffixlen, 0, 1);
}

WLIBC_INLINE int mkostemps(char *template, int suffixlen, int flags)
{
	return wlibc_common_mkstemp(template, suffixlen, flags, 1);
}

WLIBC_INLINE char *mktemp(char *template)
{
	if (wlibc_common_mkstemp(template, 0, 0, 3) != -1) // success
	{
		return template;
	}
	return NULL;
}

WLIBC_INLINE char *mkdtemp(char *template)
{
	if (wlibc_common_mkstemp(template, 0, 0, 2) != -1) // success
	{
		return template;
	}
	return NULL;
}

_WLIBC_END_DECLS

#endif
