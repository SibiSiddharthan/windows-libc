/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT
*/

#include <unistd.h>
#include <Windows.h>
#include <wlibc_errors.h>

char *wlibc_getcwd(char *buf, size_t size)
{
	if (buf == NULL)
	{
		errno = EFAULT;
		return NULL;
	}

	if (size == 0)
	{
		errno = EINVAL;
		return NULL;
	}

	DWORD length = GetCurrentDirectoryA(size, buf);
	if (length > size)
	{
		errno = ERANGE;
		return NULL;
	}

	// Convert backslash to forward slash
	char *temp = buf;
	while (*buf != '\0')
	{
		if (*buf == '\\')
		{
			*buf = '/';
		}
		++buf;
	}
	buf = temp;

	return buf;
}

wchar_t *wlibc_wgetcwd(wchar_t *wbuf, size_t size)
{
	if (wbuf == NULL)
	{
		errno = EFAULT;
		return NULL;
	}

	if (size == 0)
	{
		errno = EINVAL;
		return NULL;
	}

	DWORD length = GetCurrentDirectoryW(size, wbuf);
	if (length > size)
	{
		errno = ERANGE;
		return NULL;
	}

	// Convert backslash to forward slash
	wchar_t *temp = wbuf;
	while (*wbuf != L'\0')
	{
		if (*wbuf == L'\\')
		{
			*wbuf = L'/';
		}
		++wbuf;
	}
	wbuf = temp;

	return wbuf;
}
