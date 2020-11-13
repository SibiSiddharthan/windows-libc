/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <unistd.h>
#include <misc.h>
#include <Windows.h>
#include <wlibc_errors.h>

int common_link(const wchar_t *wsource, const wchar_t *wtarget)
{
	if (!CreateHardLink(wtarget, wsource, NULL))
	{
		map_win32_error_to_wlibc(GetLastError());
		return -1;
	}

	return 0;
}

int wlibc_link(const char *source, const char *target)
{
	if (source == NULL || target == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	wchar_t *wsource = mb_to_wc(source);
	wchar_t *wtarget = mb_to_wc(target);
	int status = common_link(wsource, wtarget);
	free(wsource);
	free(wtarget);
	return status;
}

int wlibc_wlink(const wchar_t *wsource, const wchar_t *wtarget)
{
	if (wsource == NULL || wtarget == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	return common_link(wsource, wtarget);
}
