/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <langinfo.h>
#include <langinfo_internal.h>
#include <locale.h>
#include <errno.h>
#include <string.h>
#include <time.h>

char *wlibc_nl_langinfo(nl_item item)
{
	static int invoke_count = 0;
	++invoke_count;
	if (invoke_count % MAX_LANGINFO_INVOKE_COUNT == 0)
	{
		invoke_count = 1;
	}

	switch (item)
	{
	case CODESET:
	{
		// query the locale
		char *locale = setlocale(LC_CTYPE, NULL);
		// If the string contains utf8 return UTF-8
		if (strstr(locale, "utf8"))
		{
			return "UTF-8";
		}
		else
		{
			strncpy(__nl_langinfo_buf[invoke_count - 1], locale, MAX_LANGINFO_LENGTH);
			return __nl_langinfo_buf[invoke_count - 1];
		}
	}

	case D_T_FMT:
	{
		return "%a %b %e %H:%M:%S %Y";
	}
	case D_FMT:
	{
		return "%m/%d/%y"; // Same as %D
	}
	case T_FMT:
	{
		return "%H:%M:%S"; // Same as %T
	}
	case T_FMT_AMPM:
	{
		return "%I:%M:%S %p";
	}
	case AM_STR:
	{
		char buf[32];
		struct tm TM;
		TM.tm_hour = 6; // 6AM
		strftime(buf, 32, "%p", &TM);
		strncpy(__nl_langinfo_buf[invoke_count - 1], buf, MAX_LANGINFO_LENGTH);
		return __nl_langinfo_buf[invoke_count - 1];
	}
	case PM_STR:
	{
		char buf[32];
		struct tm TM;
		TM.tm_hour = 18; // 6PM
		strftime(buf, 32, "%p", &TM);
		strncpy(__nl_langinfo_buf[invoke_count - 1], buf, MAX_LANGINFO_LENGTH);
		return __nl_langinfo_buf[invoke_count - 1];
	}

	case DAY_1:
	case DAY_2:
	case DAY_3:
	case DAY_4:
	case DAY_5:
	case DAY_6:
	case DAY_7:
	{
		char buf[32];
		struct tm TM;
		TM.tm_wday = item - DAY_1; // 0-6
		strftime(buf, 32, "%A", &TM);
		strncpy(__nl_langinfo_buf[invoke_count - 1], buf, MAX_LANGINFO_LENGTH);
		return __nl_langinfo_buf[invoke_count - 1];
	}

	case ABDAY_1:
	case ABDAY_2:
	case ABDAY_3:
	case ABDAY_4:
	case ABDAY_5:
	case ABDAY_6:
	case ABDAY_7:
	{
		char buf[32];
		struct tm TM;
		TM.tm_wday = item - ABDAY_1; // 0-6
		strftime(buf, 32, "%a", &TM);
		strncpy(__nl_langinfo_buf[invoke_count - 1], buf, MAX_LANGINFO_LENGTH);
		return __nl_langinfo_buf[invoke_count - 1];
	}

	case MON_1:
	case MON_2:
	case MON_3:
	case MON_4:
	case MON_5:
	case MON_6:
	case MON_7:
	case MON_8:
	case MON_9:
	case MON_10:
	case MON_11:
	case MON_12:
	{
		char buf[32];
		struct tm TM;
		TM.tm_mon = item - MON_1; // 0-11
		strftime(buf, 32, "%B", &TM);
		strncpy(__nl_langinfo_buf[invoke_count - 1], buf, MAX_LANGINFO_LENGTH);
		return __nl_langinfo_buf[invoke_count - 1];
	}

	case ABMON_1:
	case ABMON_2:
	case ABMON_3:
	case ABMON_4:
	case ABMON_5:
	case ABMON_6:
	case ABMON_7:
	case ABMON_8:
	case ABMON_9:
	case ABMON_10:
	case ABMON_11:
	case ABMON_12:
	{
		char buf[32];
		struct tm TM;
		TM.tm_mon = item - ABMON_1; // 0-11
		strftime(buf, 32, "%b", &TM);
		strncpy(__nl_langinfo_buf[invoke_count - 1], buf, MAX_LANGINFO_LENGTH);
		return __nl_langinfo_buf[invoke_count - 1];
	}

	case ERA:
	case ERA_D_FMT:
	case ERA_D_T_FMT:
	case ERA_T_FMT:
	case ALT_DIGITS:
	{
		return "\0";
	}

	case RADIXCHAR:
	{
		strncpy(__nl_langinfo_buf[invoke_count - 1], localeconv()->decimal_point, MAX_LANGINFO_LENGTH);
		return __nl_langinfo_buf[invoke_count - 1];
	}
	case THOUSEP:
	{
		strncpy(__nl_langinfo_buf[invoke_count - 1], localeconv()->thousands_sep, MAX_LANGINFO_LENGTH);
		return __nl_langinfo_buf[invoke_count - 1];
	}
	case CRNCYSTR:
	{
		strncpy(__nl_langinfo_buf[invoke_count - 1], localeconv()->currency_symbol, MAX_LANGINFO_LENGTH);
		return __nl_langinfo_buf[invoke_count - 1];
	}

	case YESEXPR:
	{
		return "^[yY]";
	}
	case NOEXPR:
	{
		return "^[nN]";
	}

	default:
		errno = EINVAL;
		return NULL;
	}
}
