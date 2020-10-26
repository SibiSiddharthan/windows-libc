#include <misc.h>

char *wc_to_mb(const wchar_t *wstr)
{
	int length = wcslen(wstr) + 1;
	char *str = (char *)malloc(sizeof(char) * length);
	wcstombs(str, wstr, length);
	return str;
}

wchar_t *mb_to_wc(const char *str)
{
	int length = strlen(str) + 1;
	wchar_t *wstr = (wchar_t *)malloc(sizeof(wchar_t) * length);
	mbstowcs(wstr, str, length);
	return wstr;
}

char *mbstrcat(const char *str1, const char *str2)
{
	int str1_length = strlen(str1);
	int str2_length = strlen(str2);
	char *ret = (char *)malloc(sizeof(char) * (str1_length + str2_length + 1));
	strcpy(ret, str1);
	strcat(ret, str2);
	return ret;
}

wchar_t *wcstrcat(const wchar_t *wstr1, const wchar_t *wstr2)
{
	int wstr1_length = wcslen(wstr1);
	int wstr2_length = wcslen(wstr2);
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

void fs_to_bs(char *path)
{
	while (*path != '\0')
	{
		if (*path == '/')
			*path = '\\';
		++path;
	}
}

void wfs_to_bs(wchar_t *wpath)
{
	while (*wpath != L'\0')
	{
		if (*wpath == L'/')
			*wpath = L'\\';
		++wpath;
	}
}