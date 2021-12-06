/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

/*
 * MSVC's wchar.h is huge and include a whole bunch of uneccesary header files like corecrt_wdirect.h, corecrt_wstdlib.h, etc.
 * This is just a stub compatibility header. No functions in this file will be implemented.
 */

#ifndef WLIBC_WCHAR_H
#define WLIBC_WCHAR_H

#include <wlibc.h>
#include <corecrt.h>
#include <vcruntime_string.h>

_WLIBC_BEGIN_DECLS

#define WCHAR_MIN 0x0000
#define WCHAR_MAX 0xffff

/* Let's just stick with _ACRTIMP for linkage */

_ACRTIMP wint_t __cdecl btowc(int ch);
_ACRTIMP size_t __cdecl mbrlen(char const *ch, size_t bytes, mbstate_t *state);
_ACRTIMP size_t __cdecl mbrtowc(wchar_t *dest, char const *src, size_t bytes, mbstate_t *state);
_ACRTIMP size_t __cdecl mbsrtowcs(wchar_t *dest, char const **source, size_t count, mbstate_t *state);
_ACRTIMP size_t __cdecl wcrtomb(char *dest, wchar_t source, mbstate_t *state);
_ACRTIMP size_t __cdecl wcsrtombs(char *dest, wchar_t const **source, size_t count, mbstate_t *state);
_ACRTIMP int __cdecl wctob(wint_t wch);

/* Inline functions */

WLIBC_INLINE int __cdecl mbsinit(mbstate_t const *p)
{
	return p == NULL || p->_Wchar == 0;
}

WLIBC_INLINE wchar_t *__cdecl wmemchr(wchar_t const *wstr, wchar_t wch, size_t n)
{
	for (; 0 < n; ++wstr, --n)
		if (*wstr == wch)
			return (wchar_t *)wstr;

	return 0;
}

WLIBC_INLINE int __cdecl wmemcmp(wchar_t const *wstr1, wchar_t const *wstr2, size_t count)
{
	for (; 0 < count; ++wstr1, ++wstr2, --count)
		if (*wstr1 != *wstr2)
			return *wstr1 < *wstr2 ? -1 : 1;

	return 0;
}

WLIBC_INLINE wchar_t *__cdecl wmemcpy(wchar_t *dest, wchar_t const *source, size_t count)
{
	return (wchar_t *)memcpy(dest, source, count * sizeof(wchar_t));
}

WLIBC_INLINE wchar_t *__cdecl wmemmove(wchar_t *dest, wchar_t const *source, size_t count)
{
	return (wchar_t *)memmove(dest, source, count * sizeof(wchar_t));
}

WLIBC_INLINE wchar_t *__cdecl wmemset(wchar_t *wstr, wchar_t wch, size_t count)
{
	wchar_t *temp = wstr;
	for (; 0 < count; ++temp, --count)
	{
		*temp = wch;
	}
	return wstr;
}

#if __STDC_WANT_SECURE_LIB__

_ACRTIMP errno_t __cdecl mbsrtowcs_s(size_t *retval, wchar_t *dest, size_t size, char const **source, size_t _N, mbstate_t *state);
_ACRTIMP errno_t __cdecl wcrtomb_s(size_t *retval, char *dest, size_t bytes, wchar_t ch, mbstate_t *state);
_ACRTIMP errno_t __cdecl wcsrtombs_s(size_t *retval, char *dest, size_t bytes, wchar_t const **source, size_t size, mbstate_t *state);
_ACRTIMP errno_t __cdecl wmemcpy_s(wchar_t *dest, rsize_t destsz, wchar_t const *source, rsize_t count);
_ACRTIMP errno_t __cdecl wmemmove_s(wchar_t *dest, rsize_t destsz, wchar_t const *source, rsize_t count);

#endif

_WLIBC_END_DECLS

#endif
