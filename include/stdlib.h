/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_STDLIB_H
#define WLIBC_STDLIB_H

#include <wlibc.h>
#include <corecrt_malloc.h>
#include <corecrt_search.h>
#include <corecrt_wstdlib.h>
#include <errno.h>
#include <limits.h>
#include <wchar.h>

_CRT_BEGIN_C_HEADER

#ifndef _countof
#	define _countof __crt_countof
#endif

// Minimum and maximum macros
#define __max(a, b) (((a) > (b)) ? (a) : (b))
#define __min(a, b) (((a) < (b)) ? (a) : (b))

#ifndef __cplusplus
#	define max(a, b) (((a) > (b)) ? (a) : (b))
#	define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

_ACRTIMP void __cdecl _swab(char *source, char *destination, int size);
_ACRTIMP void __cdecl swab(char *source, char *destination, int size);

// Exit and Abort
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

_ACRTIMP __declspec(noreturn) void __cdecl exit(int code);
_ACRTIMP __declspec(noreturn) void __cdecl _exit(int code);
_ACRTIMP __declspec(noreturn) void __cdecl _Exit(int code);
_ACRTIMP __declspec(noreturn) void __cdecl quick_exit(int code);
_ACRTIMP __declspec(noreturn) void __cdecl abort(void);

// Argument values for _set_abort_behavior().
#define _WRITE_ABORT_MSG  0x1 // debug only, has no effect in release
#define _CALL_REPORTFAULT 0x2

_ACRTIMP unsigned int __cdecl _set_abort_behavior(unsigned int flags, unsigned int mask);

#ifndef _CRT_ONEXIT_T_DEFINED
#	define _CRT_ONEXIT_T_DEFINED
typedef int(__CRTDECL *_onexit_t)(void);
#endif

#if defined(_CRT_INTERNAL_NONSTDC_NAMES) && _CRT_INTERNAL_NONSTDC_NAMES
// Non-ANSI name for compatibility
#	define onexit_t _onexit_t
#endif

int __cdecl atexit(void(__cdecl *)(void));
int __cdecl at_quick_exit(void(__cdecl *)(void));
_onexit_t __cdecl _onexit(_onexit_t function);
onexit_t __cdecl onexit(onexit_t _Func);

// Global State (errno, global handlers, etc.)

// a purecall handler procedure. Never returns normally
typedef void(__cdecl *_purecall_handler)(void);
// Invalid parameter handler function pointer type
typedef void(__cdecl *_invalid_parameter_handler)(wchar_t const *, wchar_t const *, wchar_t const *, unsigned int, uintptr_t);

// Establishes a purecall handler
_VCRTIMP _purecall_handler __cdecl _set_purecall_handler(_purecall_handler _Handler);
_VCRTIMP _purecall_handler __cdecl _get_purecall_handler(void);

// Establishes an invalid parameter handler
_ACRTIMP _invalid_parameter_handler __cdecl _set_invalid_parameter_handler(_invalid_parameter_handler handler);
_ACRTIMP _invalid_parameter_handler __cdecl _get_invalid_parameter_handler(void);
_ACRTIMP _invalid_parameter_handler __cdecl _set_thread_local_invalid_parameter_handler(_invalid_parameter_handler handler);
_ACRTIMP _invalid_parameter_handler __cdecl _get_thread_local_invalid_parameter_handler(void);

// Argument values for _set_error_mode().
#define TO_DEFAULT      0
#define TO_STDERR       1
#define TO_MSGBOX       2
#define _REPORT_ERRMODE 3

_ACRTIMP int __cdecl _set_error_mode(int _Mode);

// These point to the executable module name.
_CRT_INSECURE_DEPRECATE_GLOBALS(_get_pgmptr) _ACRTIMP char **__cdecl __p__pgmptr(void);
_CRT_INSECURE_DEPRECATE_GLOBALS(_get_wpgmptr) _ACRTIMP wchar_t **__cdecl __p__wpgmptr(void);
_CRT_INSECURE_DEPRECATE_GLOBALS(_get_fmode) _ACRTIMP int *__cdecl __p__fmode(void);

#ifdef _CRT_DECLARE_GLOBAL_VARIABLES_DIRECTLY
_CRT_INSECURE_DEPRECATE_GLOBALS(_get_pgmptr) extern char *_pgmptr;
_CRT_INSECURE_DEPRECATE_GLOBALS(_get_wpgmptr) extern wchar_t *_wpgmptr;
#	ifndef _CORECRT_BUILD
_CRT_INSECURE_DEPRECATE_GLOBALS(_get_fmode) extern int _fmode;
#	endif
#else
#	define _pgmptr  (*__p__pgmptr())
#	define _wpgmptr (*__p__wpgmptr())
#	define _fmode   (*__p__fmode())
#endif

_ACRTIMP errno_t __cdecl _get_pgmptr(char **value);
_ACRTIMP errno_t __cdecl _get_wpgmptr(wchar_t **value);

_ACRTIMP errno_t __cdecl _set_fmode(int _Mode);
_ACRTIMP errno_t __cdecl _get_fmode(int *_PMode);

// Math
typedef struct _div_t
{
	int quot;
	int rem;
} div_t;

typedef struct _ldiv_t
{
	long quot;
	long rem;
} ldiv_t;

typedef struct _lldiv_t
{
	long long quot;
	long long rem;
} lldiv_t;

int __cdecl abs(int _Number);
long __cdecl labs(long _Number);
long long __cdecl llabs(long long _Number);
__int64 __cdecl _abs64(__int64 _Number);

unsigned short __cdecl _byteswap_ushort(unsigned short _Number);
unsigned long __cdecl _byteswap_ulong(unsigned long _Number);
unsigned __int64 __cdecl _byteswap_uint64(unsigned __int64 _Number);

_ACRTIMP div_t __cdecl div(int _Numerator, int _Denominator);
_ACRTIMP ldiv_t __cdecl ldiv(long _Numerator, long _Denominator);
_ACRTIMP lldiv_t __cdecl lldiv(long long _Numerator, long long _Denominator);

// These functions have declspecs in their declarations in the Windows headers,
// which cause PREfast to fire 6540.
#pragma warning(push)
#pragma warning(disable : 6540)

unsigned int __cdecl _rotl(unsigned int value, int _Shift);
unsigned long __cdecl _lrotl(unsigned long value, int _Shift);
unsigned __int64 __cdecl _rotl64(unsigned __int64 value, int _Shift);
unsigned int __cdecl _rotr(unsigned int value, int _Shift);
unsigned long __cdecl _lrotr(unsigned long value, int _Shift);
unsigned __int64 __cdecl _rotr64(unsigned __int64 value, int _Shift);

#pragma warning(pop)

// Maximum value that can be returned by the rand function:
#define RAND_MAX 0x7fff

_ACRTIMP void __cdecl srand(unsigned int _Seed);
_ACRTIMP int __cdecl rand(void);

#if defined _CRT_RAND_S || defined _CRTBLD
_ACRTIMP errno_t __cdecl rand_s(unsigned int *_RandomValue);
#endif

#ifdef __cplusplus
extern "C++"
{
	inline long abs(long const _X) throw()
	{
		return labs(_X);
	}

	inline long long abs(long long const _X) throw()
	{
		return llabs(_X);
	}

	inline ldiv_t div(long const _A1, long const _A2) throw()
	{
		return ldiv(_A1, _A2);
	}

	inline lldiv_t div(long long const _A1, long long const _A2) throw()
	{
		return lldiv(_A1, _A2);
	}
}
#endif // __cplusplus

// Structs used to fool the compiler into not generating floating point
// instructions when copying and pushing [long] double values
#define _CRT_DOUBLE_DEC

#ifndef _LDSUPPORT

#	pragma pack(push, 4)
typedef struct
{
	unsigned char ld[10];
} _LDOUBLE;
#	pragma pack(pop)

#	define _PTR_LD(x) ((unsigned char *)(&(x)->ld))

#else // _LDSUPPORT

#	pragma push_macro("long")
#	undef long
typedef long double _LDOUBLE;
#	pragma pop_macro("long")

#	define _PTR_LD(x) ((unsigned char *)(x))

#endif // _LDSUPPORT

typedef struct
{
	double x;
} _CRT_DOUBLE;

typedef struct
{
	float f;
} _CRT_FLOAT;

#pragma push_macro("long")
#undef long

typedef struct
{
	long double x;
} _LONGDOUBLE;

#pragma pop_macro("long")

#pragma pack(push, 4)
typedef struct
{
	unsigned char ld12[12];
} _LDBL12;
#pragma pack(pop)

// Narrow String to Number Conversions
_ACRTIMP double __cdecl atof(char const *str);
_CRT_JIT_INTRINSIC _ACRTIMP int __cdecl atoi(char const *str);
_ACRTIMP long __cdecl atol(char const *str);
_ACRTIMP long long __cdecl atoll(char const *str);
_ACRTIMP __int64 __cdecl _atoi64(char const *str);

_ACRTIMP double __cdecl _atof_l(char const *str, _locale_t locale);
_ACRTIMP int __cdecl _atoi_l(char const *str, _locale_t locale);
_ACRTIMP long __cdecl _atol_l(char const *str, _locale_t locale);
_ACRTIMP long long __cdecl _atoll_l(char const *str, _locale_t locale);
_ACRTIMP __int64 __cdecl _atoi64_l(char const *str, _locale_t locale);

_ACRTIMP int __cdecl _atoflt(_CRT_FLOAT *_Result, char const *str);
_ACRTIMP int __cdecl _atodbl(_CRT_DOUBLE *_Result, char *str);
_ACRTIMP int __cdecl _atoldbl(_LDOUBLE *_Result, char *str);

_ACRTIMP int __cdecl _atoflt_l(_CRT_FLOAT *_Result, char const *str, _locale_t locale);
_ACRTIMP int __cdecl _atodbl_l(_CRT_DOUBLE *_Result, char *str, _locale_t locale);
_ACRTIMP int __cdecl _atoldbl_l(_LDOUBLE *_Result, char *str, _locale_t locale);

_ACRTIMP float __cdecl strtof(char const *str, char **_EndPtr);
_ACRTIMP float __cdecl _strtof_l(char const *str, char **_EndPtr, _locale_t locale);
_ACRTIMP double __cdecl strtod(char const *str, char **_EndPtr);
_ACRTIMP double __cdecl _strtod_l(char const *str, char **_EndPtr, _locale_t locale);
_ACRTIMP long double __cdecl strtold(char const *str, char **_EndPtr);
_ACRTIMP long double __cdecl _strtold_l(char const *str, char **_EndPtr, _locale_t locale);

_ACRTIMP long __cdecl strtol(char const *str, char **_EndPtr, int radix);
_ACRTIMP long __cdecl _strtol_l(char const *str, char **_EndPtr, int radix, _locale_t locale);
_ACRTIMP long long __cdecl strtoll(char const *str, char **_EndPtr, int radix);
_ACRTIMP long long __cdecl _strtoll_l(char const *str, char **_EndPtr, int radix, _locale_t locale);
_ACRTIMP unsigned long __cdecl strtoul(char const *str, char **_EndPtr, int radix);
_ACRTIMP unsigned long __cdecl _strtoul_l(char const *str, char **_EndPtr, int radix, _locale_t locale);
_ACRTIMP unsigned long long __cdecl strtoull(char const *str, char **_EndPtr, int radix);
_ACRTIMP unsigned long long __cdecl _strtoull_l(char const *str, char **_EndPtr, int radix, _locale_t locale);
_ACRTIMP __int64 __cdecl _strtoi64(char const *str, char **_EndPtr, int radix);
_ACRTIMP __int64 __cdecl _strtoi64_l(char const *str, char **_EndPtr, int radix, _locale_t locale);
_ACRTIMP unsigned __int64 __cdecl _strtoui64(char const *str, char **_EndPtr, int radix);
_ACRTIMP unsigned __int64 __cdecl _strtoui64_l(char const *str, char **_EndPtr, int radix, _locale_t locale);

// Number to Narrow String Conversions
_ACRTIMP errno_t __cdecl _itoa_s(int value, char *buffer, size_t count, int radix);
_ACRTIMP char *__cdecl _itoa(int value, char *buffer, int radix);
_ACRTIMP char *__cdecl itoa(int value, char *buffer, int radix);

_ACRTIMP errno_t __cdecl _ltoa_s(long value, char *buffer, size_t count, int radix);
_ACRTIMP char *__cdecl _ltoa(long value, char *buffer, int radix);
_ACRTIMP char *__cdecl ltoa(long value, char *buffer, int radix);

_ACRTIMP errno_t __cdecl _ultoa_s(unsigned long value, char *buffer, size_t count, int radix);
_ACRTIMP char *__cdecl _ultoa(unsigned long value, char *buffer, int radix);
_ACRTIMP char *__cdecl ultoa(unsigned long value, char *buffer, int radix);

_ACRTIMP errno_t __cdecl _i64toa_s(__int64 value, char *buffer, size_t count, int radix);
_ACRTIMP char *__cdecl _i64toa(__int64 value, char *buffer, int radix);

_ACRTIMP errno_t __cdecl _ui64toa_s(unsigned __int64 value, char *buffer, size_t count, int radix);
_ACRTIMP char *__cdecl _ui64toa(unsigned __int64 value, char *buffer, int radix);

// _CVTBUFSIZE is the maximum size for the per-thread conversion buffer.  It
// should be at least as long as the number of digits in the largest double
// precision value (?.?e308 in IEEE arithmetic).  We will use the same size
// buffer as is used in the printf support routines.
#define _CVTBUFSIZE (309 + 40) // # of digits in max. dp value + slop

_ACRTIMP errno_t __cdecl _ecvt_s(char *buffer, size_t size, double value, int count, int *dec, int *sign);
_ACRTIMP char *__cdecl _ecvt(double value, int count, int *dec, int *sign);
_ACRTIMP char *__cdecl ecvt(double value, int count, int *dec, int *sign);

_ACRTIMP errno_t __cdecl _fcvt_s(char *buffer, size_t size, double value, int count, int *dec, int *sign);
_ACRTIMP char *__cdecl _fcvt(double value, int count, int *dec, int *sign);
_ACRTIMP char *__cdecl fcvt(double value, int count, int *dec, int *sign);

_ACRTIMP errno_t __cdecl _gcvt_s(char *buffer, size_t size, double value, int count);
_ACRTIMP char *__cdecl _gcvt(double value, int count, char *buffer);
_ACRTIMP char *__cdecl gcvt(double value, int count, char *_DstBuf);

// Multibyte String Operations and Conversions
#ifndef MB_CUR_MAX
#	if defined _CRT_DISABLE_PERFCRIT_LOCKS && !defined _DLL
#		define MB_CUR_MAX __mb_cur_max
#	else
#		define MB_CUR_MAX ___mb_cur_max_func()
#	endif

#	ifdef _CRT_DECLARE_GLOBAL_VARIABLES_DIRECTLY
extern int __mb_cur_max;
#	else
#		define __mb_cur_max (___mb_cur_max_func())
#	endif

_ACRTIMP int __cdecl ___mb_cur_max_func(void);
_ACRTIMP int __cdecl ___mb_cur_max_l_func(_locale_t locale);
#endif

_ACRTIMP int __cdecl mblen(char const *_Ch, size_t count);

_ACRTIMP int __cdecl _mblen_l(char const *_Ch, size_t count, _locale_t locale);

_ACRTIMP size_t __cdecl _mbstrlen(char const *str);
_ACRTIMP size_t __cdecl _mbstrlen_l(char const *str, _locale_t locale);
_ACRTIMP size_t __cdecl _mbstrnlen(char const *str, size_t count);
_ACRTIMP size_t __cdecl _mbstrnlen_l(char const *str, size_t count, _locale_t locale);

_ACRTIMP int __cdecl mbtowc(wchar_t *destination, char const *source, size_t size);
_ACRTIMP int __cdecl _mbtowc_l(wchar_t *destination, char const *source, size_t size, _locale_t locale);

_ACRTIMP size_t __cdecl mbstowcs(wchar_t *destination, char const *source, size_t count);
_ACRTIMP size_t __cdecl _mbstowcs_l(wchar_t *destination, char const *source, size_t count, _locale_t locale);
_ACRTIMP errno_t __cdecl mbstowcs_s(size_t *converted, wchar_t *destination, size_t size, char const *source, size_t count);
_ACRTIMP errno_t __cdecl _mbstowcs_s_l(size_t *converted, wchar_t *destination, size_t size, char const *source, size_t count,
									   _locale_t locale);

_ACRTIMP int __cdecl wctomb(char *ch, wchar_t wc);
_ACRTIMP int __cdecl _wctomb_l(char *ch, wchar_t wc, _locale_t locale);
_ACRTIMP errno_t __cdecl wctomb_s(int *converted, char *ch, rsize_t size, wchar_t wc);
_ACRTIMP errno_t __cdecl _wctomb_s_l(int *converted, char *ch, size_t size, wchar_t wc, _locale_t locale);

_ACRTIMP size_t __cdecl wcstombs(char *destination, wchar_t const *source, size_t count);
_ACRTIMP errno_t __cdecl wcstombs_s(size_t *converted, char *destination, size_t size, wchar_t const *source, size_t count);
_ACRTIMP errno_t __cdecl _wcstombs_s_l(size_t *converted, char *destination, size_t size, wchar_t const *source, size_t count,
									   _locale_t locale);
_ACRTIMP size_t __cdecl _wcstombs_l(char *destination, wchar_t const *source, size_t count, _locale_t locale);

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// Path Manipulation
//
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// Sizes for buffers used by the _makepath() and _splitpath() functions.
// note that the sizes include space for 0-terminator
#define _MAX_PATH  260 // max. length of full pathname
#define _MAX_DRIVE 3   // max. length of drive component
#define _MAX_DIR   256 // max. length of path component
#define _MAX_FNAME 256 // max. length of file name component
#define _MAX_EXT   256 // max. length of extension component

#pragma push_macro("_fullpath")
#undef _fullpath

_ACRTIMP _CRTALLOCATOR char *__cdecl _fullpath(char *buffer, char const *_Path, size_t count);

#pragma pop_macro("_fullpath")

_ACRTIMP errno_t __cdecl _makepath_s(char *buffer, size_t count, char const *drive, char const *directory, char const *filename,
									 char const *extension);

_ACRTIMP void __cdecl _makepath(char *buffer, char const *drive, char const *directory, char const *filename, char const *extension);

_ACRTIMP errno_t __cdecl _splitpath_s(char const *fullpath, char *drive, size_t drive_count, char *directory, size_t directory_count,
									  char *filename, size_t filename_count, char *extension, size_t extension_count);

_ACRTIMP void __cdecl _splitpath(char const *fullpath, char *drive, char *directory, char *filename, char *extension);

_ACRTIMP int *__cdecl __p___argc(void);
_ACRTIMP char ***__cdecl __p___argv(void);
_ACRTIMP wchar_t ***__cdecl __p___wargv(void);

#define __argc  (*__p___argc())  // Pointer to number of command line arguments
#define __argv  (*__p___argv())  // Pointer to table of narrow command line arguments
#define __wargv (*__p___wargv()) // Pointer to table of wide command line arguments

_DCRTIMP char ***__cdecl __p__environ(void);
_DCRTIMP wchar_t ***__cdecl __p__wenviron(void);

#define _environ  (*__p__environ())  // Pointer to narrow environment table
#define _wenviron (*__p__wenviron()) // Pointer to wide environment table

#define environ _environ

// Sizes for buffers used by the getenv/putenv family of functions.
#define _MAX_ENV 32767

#if _CRT_FUNCTIONS_REQUIRED

_DCRTIMP char *__cdecl getenv(char const *variable);
_DCRTIMP errno_t __cdecl getenv_s(size_t *length, char *buffer, rsize_t count, char const *variable);

#	if defined(_DEBUG) && defined(_CRTDBG_MAP_ALLOC)
#		pragma push_macro("_dupenv_s")
#		undef _dupenv_s
#	endif

_DCRTIMP errno_t __cdecl _dupenv_s(char **buffer, size_t *count, char const *variable);

#	if defined(_DEBUG) && defined(_CRTDBG_MAP_ALLOC)
#		pragma pop_macro("_dupenv_s")
#	endif

_DCRTIMP int __cdecl system(char const *command);

// The functions below have declspecs in their declarations in the Windows
// headers, causing PREfast to fire 6540 here
#	pragma warning(push)
#	pragma warning(disable : 6540)

_DCRTIMP int __cdecl putenv(char const *env);
_DCRTIMP int __cdecl _putenv(char const *env);
_DCRTIMP errno_t __cdecl _putenv_s(char const *name, char const *value);

#	pragma warning(pop)

_DCRTIMP errno_t __cdecl _searchenv_s(char const *file, char const *variable, char *buffer, size_t count);
_DCRTIMP void __cdecl _searchenv(char const *file, char const *variable, char *buffer);

// The Win32 API SetErrorMode, Beep and Sleep should be used instead.
_CRT_OBSOLETE(SetErrorMode)
_DCRTIMP void __cdecl _seterrormode(int _Mode);

_CRT_OBSOLETE(Beep)
_DCRTIMP void __cdecl _beep(unsigned _Frequency, unsigned _Duration);

_CRT_OBSOLETE(Sleep)
_DCRTIMP void __cdecl _sleep(unsigned long _Duration);

#endif // _CRT_FUNCTIONS_REQUIRED

_CRT_END_C_HEADER

// Extensions
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

WLIBC_INLINE void *reallocarray(void *block, size_t count, size_t size)
{
	// Don't bother with overflow.
	return realloc(block, size * count);
}

_WLIBC_END_DECLS

#endif
