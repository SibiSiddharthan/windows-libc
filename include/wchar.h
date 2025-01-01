/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

/*
 * MSVC's wchar.h is huge and include a whole bunch of uneccesary header files like corecrt_wdirect.h, corecrt_wstdlib.h, etc.
 * This is just a stub compatibility header.
 */

#ifndef WLIBC_WCHAR_H
#define WLIBC_WCHAR_H

#include <wlibc.h>
#include <vcruntime_string.h>

#define WCHAR_MIN 0x0000
#define WCHAR_MAX 0xffff

_ACRTIMP wint_t __cdecl btowc(int ch);
_ACRTIMP size_t __cdecl mbrlen(char const *ch, size_t bytes, mbstate_t *state);
_ACRTIMP size_t __cdecl mbrtowc(wchar_t *destination, char const *src, size_t bytes, mbstate_t *state);
_ACRTIMP size_t __cdecl mbsrtowcs(wchar_t *destination, char const **source, size_t count, mbstate_t *state);
_ACRTIMP size_t __cdecl wcrtomb(char *destination, wchar_t source, mbstate_t *state);
_ACRTIMP size_t __cdecl wcsrtombs(char *destination, wchar_t const **source, size_t count, mbstate_t *state);
_ACRTIMP int __cdecl wctob(wint_t wch);

/* Inline functions */

static __inline int __cdecl mbsinit(mbstate_t const *p)
{
	return p == NULL || p->_Wchar == 0;
}

static __inline wchar_t *__cdecl wmemchr(wchar_t const *wstr, wchar_t wch, size_t n)
{
	for (; 0 < n; ++wstr, --n)
		if (*wstr == wch)
			return (wchar_t *)wstr;

	return 0;
}

static __inline int __cdecl wmemcmp(wchar_t const *wstr1, wchar_t const *wstr2, size_t count)
{
	for (; 0 < count; ++wstr1, ++wstr2, --count)
		if (*wstr1 != *wstr2)
			return *wstr1 < *wstr2 ? -1 : 1;

	return 0;
}

static __inline wchar_t *__cdecl wmemcpy(wchar_t *destination, wchar_t const *source, size_t count)
{
	return (wchar_t *)memcpy(destination, source, count * sizeof(wchar_t));
}

static __inline wchar_t *__cdecl wmemmove(wchar_t *destination, wchar_t const *source, size_t count)
{
	return (wchar_t *)memmove(destination, source, count * sizeof(wchar_t));
}

static __inline wchar_t *__cdecl wmemset(wchar_t *wstr, wchar_t wch, size_t count)
{
	wchar_t *temp = wstr;
	for (; 0 < count; ++temp, --count)
	{
		*temp = wch;
	}
	return wstr;
}

#if __STDC_WANT_SECURE_LIB__

_ACRTIMP errno_t __cdecl mbsrtowcs_s(size_t *retval, wchar_t *destination, size_t size, char const **source, size_t count,
									 mbstate_t *state);
_ACRTIMP errno_t __cdecl wcrtomb_s(size_t *retval, char *destination, size_t bytes, wchar_t ch, mbstate_t *state);
_ACRTIMP errno_t __cdecl wcsrtombs_s(size_t *retval, char *destination, size_t bytes, wchar_t const **source, size_t size,
									 mbstate_t *state);
_ACRTIMP errno_t __cdecl wmemcpy_s(wchar_t *destination, rsize_t destinationsz, wchar_t const *source, rsize_t count);
_ACRTIMP errno_t __cdecl wmemmove_s(wchar_t *destination, rsize_t destinationsz, wchar_t const *source, rsize_t count);

#endif

/* Wide string functions */

_ACRTIMP wchar_t *__cdecl wcscat(wchar_t *destination, wchar_t const *source);
_ACRTIMP wchar_t *__cdecl wcsncat(wchar_t *destination, wchar_t const *source, size_t count);
_ACRTIMP errno_t __cdecl wcscat_s(wchar_t *destination, rsize_t size, wchar_t const *source);
_ACRTIMP errno_t __cdecl wcsncat_s(wchar_t *destination, rsize_t size, wchar_t const *source, rsize_t count);

_ACRTIMP wchar_t *__cdecl wcscpy(wchar_t *destination, wchar_t const *source);
_ACRTIMP wchar_t *__cdecl wcsncpy(wchar_t *destination, wchar_t const *source, size_t count);
_ACRTIMP errno_t __cdecl wcscpy_s(wchar_t *destination, rsize_t size, wchar_t const *source);
_ACRTIMP errno_t __cdecl wcsncpy_s(wchar_t *destination, rsize_t size, wchar_t const *source, rsize_t count);

_ACRTIMP wchar_t *__cdecl wcslwr(wchar_t *str);
_ACRTIMP wchar_t *__cdecl _wcslwr(wchar_t *str);
_ACRTIMP errno_t __cdecl _wcslwr_s(wchar_t *str, size_t size);
_ACRTIMP errno_t __cdecl _wcslwr_s_l(wchar_t *str, size_t size, _locale_t _Locale);

_ACRTIMP wchar_t *__cdecl wcsupr(wchar_t *str);
_ACRTIMP wchar_t *__cdecl _wcsupr(wchar_t *str);
_ACRTIMP errno_t __cdecl _wcsupr_s(wchar_t *str, size_t size);
_ACRTIMP errno_t __cdecl _wcsupr_s_l(wchar_t *str, size_t size, _locale_t _Locale);

_ACRTIMP wchar_t *__cdecl wcsset(wchar_t *str, wchar_t value);
_ACRTIMP wchar_t *__cdecl _wcsset(wchar_t *str, wchar_t value);
_ACRTIMP wchar_t *__cdecl wcsnset(wchar_t *str, wchar_t value, size_t count);
_ACRTIMP wchar_t *__cdecl _wcsnset(wchar_t *str, wchar_t value, size_t count);
_ACRTIMP errno_t __cdecl _wcsset_s(wchar_t *destination, size_t size, wchar_t value);
_ACRTIMP errno_t __cdecl _wcsnset_s(wchar_t *destination, size_t size, wchar_t value, size_t count);

_ACRTIMP wchar_t *__cdecl wcsrev(wchar_t *str);
_ACRTIMP wchar_t *__cdecl _wcsrev(wchar_t *str);

_ACRTIMP size_t __cdecl wcsxfrm(wchar_t *destination, wchar_t const *source, size_t count);
_ACRTIMP size_t __cdecl _wcsxfrm_l(wchar_t *destination, wchar_t const *source, size_t count, _locale_t _Locale);

_ACRTIMP size_t __cdecl wcslen(wchar_t const *str);
_ACRTIMP size_t __cdecl wcsnlen(wchar_t const *source, size_t count);

_ACRTIMP int __cdecl wcscmp(wchar_t const *str1, wchar_t const *str2);
_ACRTIMP int __cdecl wcsncmp(wchar_t const *str1, wchar_t const *str2, size_t count);
_ACRTIMP int __cdecl _wcsicmp(wchar_t const *str1, wchar_t const *str2);
_ACRTIMP int __cdecl _wcsnicmp(wchar_t const *str1, wchar_t const *str2, size_t count);
_ACRTIMP int __cdecl _wcsicmp_l(wchar_t const *str1, wchar_t const *str2, _locale_t _Locale);
_ACRTIMP int __cdecl _wcsnicmp_l(wchar_t const *str1, wchar_t const *str2, size_t count, _locale_t _Locale);

_ACRTIMP int __cdecl wcsicmp(wchar_t const *str1, wchar_t const *str2);
_ACRTIMP int __cdecl wcsnicmp(wchar_t const *str1, wchar_t const *str2, size_t count);

_ACRTIMP int __cdecl wcscoll(wchar_t const *str1, wchar_t const *str2);
_ACRTIMP int __cdecl _wcscoll_l(wchar_t const *str1, wchar_t const *str2, _locale_t _Locale);
_ACRTIMP int __cdecl _wcsicoll(wchar_t const *str1, wchar_t const *str2);
_ACRTIMP int __cdecl _wcsicoll_l(wchar_t const *str1, wchar_t const *str2, _locale_t _Locale);

_ACRTIMP int __cdecl _wcsncoll(wchar_t const *str1, wchar_t const *str2, size_t count);
_ACRTIMP int __cdecl _wcsncoll_l(wchar_t const *str1, wchar_t const *str2, size_t count, _locale_t _Locale);
_ACRTIMP int __cdecl _wcsnicoll(wchar_t const *str1, wchar_t const *str2, size_t count);
_ACRTIMP int __cdecl _wcsnicoll_l(wchar_t const *str1, wchar_t const *str2, size_t count, _locale_t _Locale);

_ACRTIMP int __cdecl wcsicoll(wchar_t const *str1, wchar_t const *str2);

_ACRTIMP size_t __cdecl wcscspn(wchar_t const *str, wchar_t const *_Control);
_ACRTIMP size_t __cdecl wcsspn(wchar_t const *str, wchar_t const *_Control);

_ACRTIMP wchar_t _CONST_RETURN *__cdecl wcspbrk(wchar_t const *str, wchar_t const *_Control);

_ACRTIMP wchar_t *__cdecl wcstok(wchar_t *str, wchar_t const *delimiter, wchar_t **context);
_ACRTIMP wchar_t *__cdecl wcstok_s(wchar_t *str, wchar_t const *delimiter, wchar_t **context);

#if defined _DEBUG && defined _CRTDBG_MAP_ALLOC
#	pragma push_macro("_wcsdup")
#	pragma push_macro("wcsdup")
#	undef _wcsdup
#	undef wcsdup
#endif

_ACRTIMP _CRTALLOCATOR wchar_t *__cdecl _wcsdup(wchar_t const *str);
_ACRTIMP _CRTALLOCATOR wchar_t *__cdecl wcsdup(wchar_t const *str);

#if defined _DEBUG && defined _CRTDBG_MAP_ALLOC
#	pragma pop_macro("wcsdup")
#	pragma pop_macro("_wcsdup")
#endif

/* Extensions */
_WLIBC_BEGIN_DECLS

WLIBC_API int wlibc_wcwidth(wchar_t wc);
WLIBC_API int wlibc_wcswidth(const wchar_t *wstr, size_t size);

WLIBC_INLINE int wcwidth(wchar_t wc)
{
	return wlibc_wcwidth(wc);
}

WLIBC_INLINE int wcswidth(const wchar_t *wstr, size_t size)
{
	return wlibc_wcswidth(wstr, size);
}

_WLIBC_END_DECLS

#endif
