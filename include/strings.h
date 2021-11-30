/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_STRINGS_H
#define WLIBC_STRINGS_H

#include <wlibc-macros.h>
#include <string.h>

_WLIBC_BEGIN_DECLS

WLIBC_INLINE int bcmp(const void *buf1, const void *buf2, size_t size)
{
	return memcmp(buf1, buf2, size);
}

WLIBC_INLINE void bcopy(const void *source, void *dest, size_t size)
{
	memmove(dest, source, size);
}

WLIBC_INLINE void bzero(void *buffer, size_t size)
{
	memset(buffer, 0, size);
}

WLIBC_INLINE char *index(const char *str, int ch)
{
	return strchr(str, ch);
}

WLIBC_INLINE char *rindex(const char *str, int ch)
{
	return strrchr(str, ch);
}

WLIBC_API int wlibc_ffs32(int bytes_32);
WLIBC_API int wlibc_ffs64(long long int bytes_64);

WLIBC_INLINE int ffs(int i)
{
	return wlibc_ffs32(i);
}

WLIBC_INLINE int ffsl(long int li)
{
	return wlibc_ffs32(li);
}

WLIBC_INLINE int ffsll(long long int lli)
{
	return wlibc_ffs64(lli);
}

WLIBC_INLINE int strcasecmp(const char *str1, const char *str2)
{
	return _stricmp(str1, str2);
}

WLIBC_INLINE int strncasecmp(const char *str1, const char *str2, size_t size)
{
	return _strnicmp(str1, str2, size);
}

WLIBC_INLINE int strcasecmp_l(const char *str1, const char *str2, _locale_t loc)
{
	return _stricmp_l(str1, str2, loc);
}

WLIBC_INLINE int strncasecmp_l(const char *str1, const char *str2, size_t size, _locale_t loc)
{
	return _strnicmp_l(str1, str2, size, loc);
}

_WLIBC_END_DECLS

#endif
