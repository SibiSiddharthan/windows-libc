/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT
*/

#include <unistd.h>
#include <wchar.h>
#include <misc.h>
#include <errno.h>
#include <Windows.h>
#include <wlibc_errors.h>
#include <stdlib.h>

int common_symlink(const wchar_t *wsource, const wchar_t *wtarget)
{
	DWORD attributes;
	DWORD flags = SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE;

	attributes = GetFileAttributes(wtarget);
	if (attributes != INVALID_FILE_ATTRIBUTES)
	{
		errno = EEXIST;
		return -1;
	}

	int wtarget_length = wcslen(wtarget);
	int last_slash_pos = 0;
	for (int i = wtarget_length - 1; i > 0; i--)
	{
		if (wtarget[i] == L'/' || wtarget[i] == L'\\')
		{
			last_slash_pos = i;
			break;
		}
	}

	// Symlink is created across directories in a relative manner
	// We find the type of file here
	int wsource_length = wcslen(wsource);
	if (last_slash_pos != 0 && !is_absolute_pathw(wsource))
	{
		wchar_t *wsource_final = (wchar_t *)malloc(sizeof(wchar_t) * (wsource_length + last_slash_pos + 2)); // '\0','/'
		wcsncpy(wsource_final, wtarget, last_slash_pos + 1);
		wsource_final[last_slash_pos + 1] = L'\0';
		wcscat(wsource_final, wsource);
		attributes = GetFileAttributes(wsource_final);
		if (attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			flags |= SYMBOLIC_LINK_FLAG_DIRECTORY;
		}
		free(wsource_final);
	}
	else
	{
		attributes = GetFileAttributes(wsource);
		if (attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			flags |= SYMBOLIC_LINK_FLAG_DIRECTORY;
		}
	}

	// wsource needs to have backslashes only
	wchar_t *wsource_bs = (wchar_t *)malloc(sizeof(wchar_t) * (wsource_length + 1));
	wcscpy(wsource_bs, wsource);

	wchar_t *temp = wsource_bs;
	while (*wsource_bs != L'\0')
	{
		if (*wsource_bs == L'/')
			*wsource_bs = L'\\';
		++wsource_bs;
	}
	wsource_bs = temp;

	if (!CreateSymbolicLink(wtarget, wsource_bs, flags))
	{
		map_win32_error_to_wlibc(GetLastError());
		return -1;
	}
	free(wsource_bs);

	return 0;
}

int wlibc_symlink(const char *source, const char *target)
{
	if (source == NULL || target == NULL)
	{
		errno = ENOENT;
		return -1;
	}
	wchar_t *wsource = mb_to_wc(source);
	wchar_t *wtarget = mb_to_wc(target);
	int status = common_symlink(wsource, wtarget);
	free(wsource);
	free(wtarget);

	return status;
}

int wlibc_wsymlink(const wchar_t *wsource, const wchar_t *wtarget)
{
	if (wsource == NULL || wtarget == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	return common_symlink(wsource, wtarget);
}