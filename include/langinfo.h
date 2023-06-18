/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_LANGINFO_H
#define WLIBC_LANGINFO_H

#include <wlibc.h>

_WLIBC_BEGIN_DECLS

typedef int nl_item; // type used by nl_langinfo

#define CODESET 0 // Codeset name.

#define D_T_FMT    1 // String for formatting date and time.
#define D_FMT      2 // Date format string.
#define T_FMT      3 // Time format string.
#define T_FMT_AMPM 4 // a.m. or p.m. time format string.

#define AM_STR 5 // Ante-meridiem affix.
#define PM_STR 6 // Post-meridiem affix.

#define DAY_1 7  // Name of the first day of the week (eg. Sunday)
#define DAY_2 8  // Name of the second day of the week
#define DAY_3 9  // Name of the third day of the week
#define DAY_4 10 // Name of the fourth day of the week
#define DAY_5 11 // Name of the fifth day of the week
#define DAY_6 12 // Name of the sixth day of the week
#define DAY_7 13 // Name of the seventh day of the week

#define ABDAY_1 14 // Abbreviated name of the first day of the week.
#define ABDAY_2 15 // Abbreviated name of the second day of the week.
#define ABDAY_3 16 // Abbreviated name of the third day of the week.
#define ABDAY_4 17 // Abbreviated name of the fourth day of the week.
#define ABDAY_5 18 // Abbreviated name of the fifth day of the week.
#define ABDAY_6 19 // Abbreviated name of the sixth day of the week.
#define ABDAY_7 20 // Abbreviated name of the seventh day of the week.

#define MON_1  21 // Name of the first month of the year.
#define MON_2  22 // Name of the second month.
#define MON_3  23 // Name of the third month.
#define MON_4  24 // Name of the fourth month.
#define MON_5  25 // Name of the fifth month.
#define MON_6  26 // Name of the sixth month.
#define MON_7  27 // Name of the seventh month.
#define MON_8  28 // Name of the eighth month.
#define MON_9  29 // Name of the ninth month.
#define MON_10 30 // Name of the tenth month.
#define MON_11 31 // Name of the eleventh month.
#define MON_12 32 // Name of the twelfth month.

#define ABMON_1  33 // Abbreviated name of the first month.
#define ABMON_2  34 // Abbreviated name of the second month.
#define ABMON_3  35 // Abbreviated name of the third month.
#define ABMON_4  36 // Abbreviated name of the fourth month.
#define ABMON_5  37 // Abbreviated name of the fifth month.
#define ABMON_6  38 // Abbreviated name of the sixth month.
#define ABMON_7  39 // Abbreviated name of the seventh month.
#define ABMON_8  40 // Abbreviated name of the eighth month.
#define ABMON_9  41 // Abbreviated name of the ninth month.
#define ABMON_10 42 // Abbreviated name of the tenth month.
#define ABMON_11 43 // Abbreviated name of the eleventh month.
#define ABMON_12 44 // Abbreviated name of the twelfth month.

#define ERA         45 // Era description segments.
#define ERA_D_FMT   46 // Era date format string.
#define ERA_D_T_FMT 47 // Era date and time format string.
#define ERA_T_FMT   48 // Era time format string.

#define ALT_DIGITS 49 // Alternative symbols for digits.
#define RADIXCHAR  50 // Radix character.
#define THOUSEP    51 // Separator for thousands.
#define CRNCYSTR   52 // Currency Symbol

#define YESEXPR 53 // Affirmative response expression.
#define NOEXPR  54 // Negative response expression.

WLIBC_API char *wlibc_nl_langinfo(nl_item item);

WLIBC_INLINE char *nl_langinfo(nl_item item)
{
	return wlibc_nl_langinfo(item);
}

_WLIBC_END_DECLS

#endif
