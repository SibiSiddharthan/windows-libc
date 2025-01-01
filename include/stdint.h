/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_STDINT_H
#define WLIBC_STDINT_H

// clang-format off

typedef signed char        int8_t;
typedef short              int16_t;
typedef int                int32_t;
typedef long long          int64_t;
typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

typedef signed char        int_least8_t;
typedef short              int_least16_t;
typedef int                int_least32_t;
typedef long long          int_least64_t;
typedef unsigned char      uint_least8_t;
typedef unsigned short     uint_least16_t;
typedef unsigned int       uint_least32_t;
typedef unsigned long long uint_least64_t;

typedef signed char        int_fast8_t;
typedef int                int_fast16_t;
typedef int                int_fast32_t;
typedef long long          int_fast64_t;
typedef unsigned char      uint_fast8_t;
typedef unsigned int       uint_fast16_t;
typedef unsigned int       uint_fast32_t;
typedef unsigned long long uint_fast64_t;

typedef long long          intmax_t;
typedef unsigned long long uintmax_t;

// These macros should match intsafe.h.
#define INT8_MIN         (-127i8 - 1)
#define INT16_MIN        (-32767i16 - 1)
#define INT32_MIN        (-2147483647i32 - 1)
#define INT64_MIN        (-9223372036854775807i64 - 1)
#define INT8_MAX         127i8
#define INT16_MAX        32767i16
#define INT32_MAX        2147483647i32
#define INT64_MAX        9223372036854775807i64
#define UINT8_MAX        0xffui8
#define UINT16_MAX       0xffffui16
#define UINT32_MAX       0xffffffffui32
#define UINT64_MAX       0xffffffffffffffffui64

#define INT_LEAST8_MIN   INT8_MIN
#define INT_LEAST16_MIN  INT16_MIN
#define INT_LEAST32_MIN  INT32_MIN
#define INT_LEAST64_MIN  INT64_MIN
#define INT_LEAST8_MAX   INT8_MAX
#define INT_LEAST16_MAX  INT16_MAX
#define INT_LEAST32_MAX  INT32_MAX
#define INT_LEAST64_MAX  INT64_MAX
#define UINT_LEAST8_MAX  UINT8_MAX
#define UINT_LEAST16_MAX UINT16_MAX
#define UINT_LEAST32_MAX UINT32_MAX
#define UINT_LEAST64_MAX UINT64_MAX

#define INT_FAST8_MIN    INT8_MIN
#define INT_FAST16_MIN   INT32_MIN
#define INT_FAST32_MIN   INT32_MIN
#define INT_FAST64_MIN   INT64_MIN
#define INT_FAST8_MAX    INT8_MAX
#define INT_FAST16_MAX   INT32_MAX
#define INT_FAST32_MAX   INT32_MAX
#define INT_FAST64_MAX   INT64_MAX
#define UINT_FAST8_MAX   UINT8_MAX
#define UINT_FAST16_MAX  UINT32_MAX
#define UINT_FAST32_MAX  UINT32_MAX
#define UINT_FAST64_MAX  UINT64_MAX

#ifdef _WIN64
	#define INTPTR_MIN   INT64_MIN
	#define INTPTR_MAX   INT64_MAX
	#define UINTPTR_MAX  UINT64_MAX
#else
	#define INTPTR_MIN   INT32_MIN
	#define INTPTR_MAX   INT32_MAX
	#define UINTPTR_MAX  UINT32_MAX
#endif

#define INTMAX_MIN       INT64_MIN
#define INTMAX_MAX       INT64_MAX
#define UINTMAX_MAX      UINT64_MAX

#define PTRDIFF_MIN      INTPTR_MIN
#define PTRDIFF_MAX      INTPTR_MAX

#ifndef SIZE_MAX
	#ifdef _WIN64
		#define SIZE_MAX 0xffffffffffffffffui64
	#else
		#define SIZE_MAX 0xffffffffui32
	#endif
#endif

#define SIG_ATOMIC_MIN   INT32_MIN
#define SIG_ATOMIC_MAX   INT32_MAX

#define WCHAR_MIN        0x0000
#define WCHAR_MAX        0xffff

#define WINT_MIN         0x0000
#define WINT_MAX         0xffff

#define INT8_C(x)    (x)
#define INT16_C(x)   (x)
#define INT32_C(x)   (x)
#define INT64_C(x)   (x ## LL)

#define UINT8_C(x)   (x)
#define UINT16_C(x)  (x)
#define UINT32_C(x)  (x ## U)
#define UINT64_C(x)  (x ## ULL)

#define INTMAX_C(x)  INT64_C(x)
#define UINTMAX_C(x) UINT64_C(x)


#define INT8_WIDTH         8
#define UINT8_WIDTH        8
#define INT16_WIDTH        16
#define UINT16_WIDTH       16
#define INT32_WIDTH        32
#define UINT32_WIDTH       32
#define INT64_WIDTH        64
#define UINT64_WIDTH       64

#define INT_LEAST8_WIDTH   8
#define UINT_LEAST8_WIDTH  8
#define INT_LEAST16_WIDTH  16
#define UINT_LEAST16_WIDTH 16
#define INT_LEAST32_WIDTH  32
#define UINT_LEAST32_WIDTH 32
#define INT_LEAST64_WIDTH  64
#define UINT_LEAST64_WIDTH 64

#define INT_FAST8_WIDTH    8
#define UINT_FAST8_WIDTH   8
#define INT_FAST16_WIDTH   32
#define UINT_FAST16_WIDTH  32
#define INT_FAST32_WIDTH   32
#define UINT_FAST32_WIDTH  32
#define INT_FAST64_WIDTH   64
#define UINT_FAST64_WIDTH  64

#ifdef _WIN64
#	define INTPTR_WIDTH    INT64_WIDTH
#	define UINTPTR_WIDTH   INT64_WIDTH
#	define PTRDIFF_WIDTH   INT64_WIDTH
#	define SIZE_WIDTH      INT64_WIDTH
#else
#	define INTPTR_WIDTH    INT32_WIDTH
#	define UINTPTR_WIDTH   INT32_WIDTH
#	define PTRDIFF_WIDTH   INT32_WIDTH
#	define SIZE_WIDTH      INT32_WIDTH
#endif

#define INTMAX_WIDTH       64
#define UINTMAX_WIDTH      64

#define SIG_ATOMIC_WIDTH   32
#define WCHAR_WIDTH        32
#define WINT_WIDTH         32

// clang-format on

#endif
