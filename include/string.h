/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_STRING_EXT_H
#define WLIBC_STRING_EXT_H

#include <wlibc.h>
#include <corecrt_memory.h>

#define _NLSCMPERROR _CRT_INT_MAX // currently == INT_MAX

/* String manipulation */

_ACRTIMP char *__cdecl strcat(char *destination, char const *source);
_ACRTIMP char *__cdecl strncat(char *destination, char const *source, size_t count);
_ACRTIMP errno_t __cdecl strcat_s(char *destination, rsize_t size, char const *source);
_ACRTIMP errno_t __cdecl strncat_s(char *destination, rsize_t size, char const *source, rsize_t maxsize);

_ACRTIMP char *__cdecl strcpy(char *destination, char const *source);
_ACRTIMP char *__cdecl strncpy(char *destination, char const *source, size_t count);
_ACRTIMP errno_t __cdecl strcpy_s(char *destination, rsize_t size, char const *source);
_ACRTIMP errno_t __cdecl strncpy_s(char *destination, rsize_t size, char const *source, rsize_t maxsize);

_ACRTIMP char *__cdecl strlwr(char *str);
_ACRTIMP char *__cdecl _strlwr(char *str);
_ACRTIMP errno_t __cdecl _strlwr_s(char *str, size_t size);
_ACRTIMP errno_t __cdecl _strlwr_s_l(char *str, size_t size, _locale_t locale);

_ACRTIMP char *__cdecl strupr(char *str);
_ACRTIMP char *__cdecl _strupr(char *str);
_ACRTIMP errno_t __cdecl _strupr_s(char *str, size_t size);
_ACRTIMP errno_t __cdecl _strupr_s_l(char *str, size_t size, _locale_t locale);

_ACRTIMP char *__cdecl strset(char *str, int value);
_ACRTIMP char *__cdecl strnset(char *str, int value, size_t count);
_ACRTIMP char *__cdecl _strset(char *str, int value);
_ACRTIMP char *__cdecl _strnset(char *str, int value, size_t count);
_ACRTIMP errno_t __cdecl _strset_s(char *destination, size_t destination_size, int value);
_ACRTIMP errno_t __cdecl _strnset_s(char *str, size_t size, int value, size_t count);

_ACRTIMP char *__cdecl strrev(char *str);
_ACRTIMP char *__cdecl _strrev(char *str);

_ACRTIMP size_t __cdecl strxfrm(char *destination, char const *source, size_t count);
_ACRTIMP size_t __cdecl _strxfrm_l(char *destination, char const *source, size_t count, _locale_t locale);

/* String examination */

_ACRTIMP size_t __cdecl strlen(char const *str);
_ACRTIMP size_t __cdecl strnlen(char const *str, size_t count);
static __inline size_t __CRTDECL strnlen_s(char const *str, size_t size)
{
	return str == NULL ? 0 : strnlen(str, size);
}

_ACRTIMP int __cdecl strcmp(char const *str1, char const *str2);
_ACRTIMP int __cdecl stricmp(char const *str1, char const *str2);
_ACRTIMP int __cdecl _stricmp(char const *str1, char const *str2);
_ACRTIMP int __cdecl _stricmp_l(char const *str1, char const *str2, _locale_t locale);

_ACRTIMP int __cdecl strncmp(char const *str1, char const *str2, size_t count);
_ACRTIMP int __cdecl strnicmp(char const *str1, char const *str2, size_t count);
_ACRTIMP int __cdecl _strnicmp(char const *str1, char const *str2, size_t count);
_ACRTIMP int __cdecl _strnicmp_l(char const *str1, char const *str2, size_t count, _locale_t locale);

_ACRTIMP int __cdecl strcmpi(char const *str1, char const *str2);
_ACRTIMP int __cdecl _strcmpi(char const *str1, char const *str2);

_ACRTIMP int __cdecl strcoll(char const *str1, char const *str2);
_ACRTIMP int __cdecl _strcoll_l(char const *str1, char const *str2, _locale_t locale);
_ACRTIMP int __cdecl _stricoll(char const *str1, char const *str2);
_ACRTIMP int __cdecl _stricoll_l(char const *str1, char const *str2, _locale_t locale);

_ACRTIMP int __cdecl _strncoll(char const *str1, char const *str2, size_t count);
_ACRTIMP int __cdecl _strncoll_l(char const *str1, char const *str2, size_t count, _locale_t locale);
_ACRTIMP int __cdecl _strnicoll(char const *str1, char const *str2, size_t count);
_ACRTIMP int __cdecl _strnicoll_l(char const *str1, char const *str2, size_t count, _locale_t locale);

_ACRTIMP size_t __cdecl strspn(char const *str, char const *control);
_ACRTIMP size_t __cdecl strcspn(char const *str, char const *control);

_ACRTIMP char *__cdecl strpbrk(char const *str, char const *control);

_ACRTIMP char *__cdecl strtok(_Inout_opt_ char *str, char const *delimiter);
_ACRTIMP char *__cdecl strtok_s(char *str, char const *delimiter, char **context);

_ACRTIMP size_t __cdecl __strncnt(char const *str, size_t count);

#if defined _DEBUG && defined _CRTDBG_MAP_ALLOC
#	pragma push_macro("_strdup")
#	pragma push_macro("strdup")
#	undef _strdup
#	undef strdup
#endif

_ACRTIMP _CRTALLOCATOR char *__cdecl _strdup(char const *source);
_ACRTIMP _CRTALLOCATOR char *__cdecl strdup(char const *source);

#if defined _DEBUG && defined _CRTDBG_MAP_ALLOC
#	pragma pop_macro("strdup")
#	pragma pop_macro("_strdup")
#endif

/* Multibyte string functions */
#define _FILE_DEFINED
#include <mbstring.h>

/* Extensions */
_WLIBC_BEGIN_DECLS

/* Inline implementations */

WLIBC_INLINE char *stpcpy(char *destination, const char *source)
{
	return strcpy(destination, source) + strlen(source);
}

WLIBC_INLINE char *stpncpy(char *destination, const char *source, size_t size)
{
	return strncpy(destination, source, size) + strnlen(source, size);
}

WLIBC_INLINE int strcasecmp(char const *str1, char const *str2)
{
	return _stricmp(str1, str2);
}

WLIBC_INLINE int strcasecoll(char const *str1, char const *str2)
{
	return _stricoll(str1, str2);
}

WLIBC_INLINE int stricoll(char const *str1, char const *str2)
{
	return _stricoll(str1, str2);
}

WLIBC_INLINE mbslen(unsigned char const *str)
{
	return _mbslen(str);
}

WLIBC_INLINE char *mbschr(unsigned char const *str, unsigned int ch)
{
	return _mbschr(str, ch);
}

WLIBC_INLINE char *mbsstr(unsigned char const *str, unsigned char const *sub)
{
	return _mbsstr(str, sub);
}

WLIBC_INLINE mbscasecmp(unsigned char const *str1, unsigned char const *str2)
{
	return _mbsicmp(str1, str2);
}

WLIBC_INLINE void *mempcpy(void *destination, const void *source, size_t size)
{
	return (void *)((char *)memcpy(destination, source, size) + size);
}

WLIBC_INLINE void *rawmemchr(const void *str, int character)
{
	return memchr(str, character, -1ull);
}

/* API implementations */

WLIBC_API char *wlibc_common_strerror(int errnum, _locale_t locale);

WLIBC_INLINE char *strerror(int errnum)
{
	return wlibc_common_strerror(errnum, NULL);
}

#pragma warning(push)
#pragma warning(disable : 4100) // Unused parameter

WLIBC_INLINE char *strerror_r(int errnum, char *buffer WLIBC_UNUSED, size_t length WLIBC_UNUSED)
{
	return wlibc_common_strerror(errnum, NULL);
}

#pragma warning(pop)

WLIBC_INLINE char *strerror_l(int errnum, _locale_t locale)
{
	return wlibc_common_strerror(errnum, locale);
}

WLIBC_API char *wlibc_strndup(const char *str, size_t size);
WLIBC_INLINE char *strndup(const char *str, size_t size)
{
	return wlibc_strndup(str, size);
}

WLIBC_API char *wlibc_strsignal(int sig);
WLIBC_INLINE char *strsignal(int sig)
{
	return wlibc_strsignal(sig);
}

WLIBC_API void *wlibc_memrchr(const void *str, int character, size_t size);
WLIBC_INLINE void *memrchr(const void *str, int character, size_t size)
{
	return (void *)wlibc_memrchr(str, character, size);
}

_WLIBC_END_DECLS

/* Extra stuff */
#include <strings.h>

#endif
