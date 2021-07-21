/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <unistd.h>
#include <misc.h>
#include <Windows.h>
#include <wlibc_errors.h>
#include <stdlib.h>

// Convert backslash to forward slash
void convert_bs_to_fs(wchar_t *str, int length)
{
	for (int i = 0; i < length; i++)
	{
		if (str[i] == L'\\')
		{
			str[i] = L'/';
		}
	}
}

ssize_t common_readlink(const wchar_t *wpath, wchar_t *wbuf, size_t bufsiz, int give_absolute)
{
	if (bufsiz < 1)
	{
		errno = EINVAL;
		return -1;
	}
	int bufsiz_fill = 0;

	HANDLE h_real = CreateFile(wpath, FILE_READ_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING,
							   FILE_FLAG_BACKUP_SEMANTICS | FILE_ATTRIBUTE_NORMAL, NULL);
	if (h_real == INVALID_HANDLE_VALUE)
	{
		map_win32_error_to_wlibc(GetLastError());
		return -1;
	}

	HANDLE h_sym = CreateFile(wpath, FILE_READ_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING,
							  FILE_FLAG_BACKUP_SEMANTICS | FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OPEN_REPARSE_POINT, NULL);
	if (h_sym == INVALID_HANDLE_VALUE)
	{
		map_win32_error_to_wlibc(GetLastError());
		CloseHandle(h_real);
		return -1;
	}

	wchar_t path_real[MAX_PATH], path_sym[MAX_PATH];

	DWORD length_real = GetFinalPathNameByHandle(h_real, path_real, MAX_PATH, FILE_NAME_NORMALIZED | VOLUME_NAME_NONE);
	if (length_real == 0)
	{
		map_win32_error_to_wlibc(GetLastError());
		CloseHandle(h_real);
		CloseHandle(h_sym);
		return -1;
	}

	DWORD length_sym = GetFinalPathNameByHandle(h_sym, path_sym, MAX_PATH, FILE_NAME_NORMALIZED | VOLUME_NAME_NONE);
	if (length_sym == 0)
	{
		map_win32_error_to_wlibc(GetLastError());
		CloseHandle(h_real);
		CloseHandle(h_sym);
		return -1;
	}

	CloseHandle(h_real);
	CloseHandle(h_sym);

	if (wcscmp(path_real, path_sym) == 0)
	{
		errno = EINVAL;
		return -1;
	}

	if (give_absolute)
	{
		wcsncpy(wbuf, path_real, length_real);
		convert_bs_to_fs(wbuf, length_real);
		return length_real;
	}

	int max_length = length_real >= length_sym ? length_real : length_sym;
	int point_of_change = -1;
	int last_slash_pos = 0;
	for (int i = 0; i < max_length; i++)
	{
		if (path_sym[i] == L'\\')
		{
			last_slash_pos = i;
		}
		if (path_real[i] != path_sym[i])
		{
			point_of_change = last_slash_pos + 1;
			break;
		}
	}

	if (point_of_change != -1)
	{
		int num_of_slashes = 0;
		for (int i = point_of_change; i < max_length; i++)
		{
			if (path_sym[i] == L'\\')
			{
				++num_of_slashes;
			}
		}

		for (int i = 0; i < num_of_slashes; i++)
		{
			if ((bufsiz - bufsiz_fill) > 3)
			{
				wcsncpy(wbuf + bufsiz_fill, L"../", 3);
				bufsiz_fill += 3;
			}
			else
			{
				wcsncpy(wbuf + bufsiz_fill, L"../", bufsiz - bufsiz_fill);
				bufsiz_fill = bufsiz;
			}
		}

		int remaining_length =
			(length_real - point_of_change) < (bufsiz - bufsiz_fill) ? (length_real - point_of_change) : (bufsiz - bufsiz_fill);
		wcsncpy(wbuf + bufsiz_fill, path_real + point_of_change, remaining_length);
		bufsiz_fill += remaining_length;
	}

	// Convert backslash to forward slash
	convert_bs_to_fs(wbuf, bufsiz_fill);

	return bufsiz_fill;
}

ssize_t wlibc_readlink(const char *path, char *buf, size_t bufsiz)
{
	if (path == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	wchar_t *wpath = mb_to_wc(path);
	wchar_t *wbuf = (wchar_t *)malloc(sizeof(wchar_t) * bufsiz);
	ssize_t length = common_readlink(wpath, wbuf, bufsiz, 0);
	if (length != -1)
	{
		wcstombs(buf, wbuf, length);
	}
	free(wbuf);
	free(wpath);
	return length;
}

ssize_t wlibc_wreadlink(const wchar_t *wpath, wchar_t *wbuf, size_t bufsiz)
{
	if (wpath == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	return common_readlink(wpath, wbuf, bufsiz, 0);
}
