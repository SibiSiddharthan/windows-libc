/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <unistd.h>
#include <Windows.h>
#include <internal/error.h>

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
