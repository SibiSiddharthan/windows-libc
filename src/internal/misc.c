/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/misc.h>

char *wc_to_mb(const wchar_t *wstr)
{
	size_t length = wcslen(wstr) + 1;
	char *str = (char *)malloc(sizeof(char) * length);
	wcstombs(str, wstr, length);
	return str;
}

wchar_t *mb_to_wc(const char *str)
{
	size_t length = strlen(str) + 1;
	wchar_t *wstr = (wchar_t *)malloc(sizeof(wchar_t) * length);
	mbstowcs(wstr, str, length);
	return wstr;
}

char *mbstrcat(const char *str1, const char *str2)
{
	size_t str1_length = strlen(str1);
	size_t str2_length = strlen(str2);
	char *ret = (char *)malloc(sizeof(char) * (str1_length + str2_length + 1));
	strcpy(ret, str1);
	strcat(ret, str2);
	return ret;
}

wchar_t *wcstrcat(const wchar_t *wstr1, const wchar_t *wstr2)
{
	size_t wstr1_length = wcslen(wstr1);
	size_t wstr2_length = wcslen(wstr2);
	wchar_t *ret = (wchar_t *)malloc(sizeof(wchar_t) * (wstr1_length + wstr2_length + 1));
	wcscpy(ret, wstr1);
	wcscat(ret, wstr2);
	return ret;
}

int is_absolute_path(const char *str)
{
	if (str != NULL)
	{
		if (str[0] == '\\' || str[0] == '/' || str[1] == ':') //|| str[5] == ':') // "/" ,"\", "C:" , "//?/C:"
		{
			return 1;
		}
	}
	return 0;
}

int is_absolute_pathw(const wchar_t *wstr)
{
	if (wstr != NULL)
	{
		if (wstr[0] == L'\\' || wstr[0] == L'/' || wstr[1] == L':') //|| wstr[5] == L':')
		{
			return 1;
		}
	}
	return 0;
}

int has_executable_extenstion(const wchar_t *wstr)
{
	wchar_t ext[4];
	size_t length = wcslen(wstr);
	if (length > 3)
	{
		wcsncpy(ext, wstr + (length - 4), 4);
		if (_wcsnicmp(ext, L".exe", 4) == 0 || _wcsnicmp(ext, L".cmd", 4) == 0 || _wcsnicmp(ext, L".bat", 4) == 0 ||
			_wcsnicmp(ext, L".com", 4) == 0)
		{
			return 1;
		}
	}

	return 0;
}
