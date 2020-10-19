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