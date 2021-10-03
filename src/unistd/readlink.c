/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <unistd.h>
#include <internal/misc.h>
#include <Windows.h>
#include <internal/error.h>
#include <internal/fcntl.h>
#include <internal/nt.h>
#include <stdlib.h>

// Convert backslash to forward slash
void convert_bs_to_fs(char *str, int length)
{
	for (int i = 0; i < length; i++)
	{
		if (str[i] == '\\')
		{
			str[i] = '/';
		}
	}
}

#if 0
ssize_t common_readlink(const wchar_t *restrict wpath, wchar_t *restrict wbuf, size_t bufsiz, int give_absolute)
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

ssize_t wlibc_readlink(const char *restrict path, char *restrict buf, size_t bufsiz)
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

ssize_t wlibc_wreadlink(const wchar_t *restrict wpath, wchar_t *restrict wbuf, size_t bufsiz)
{
	if (wpath == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	return common_readlink(wpath, wbuf, bufsiz, 0);
}

#endif

ssize_t do_readlink(HANDLE handle, char *restrict buf, size_t bufsiz)
{
	ssize_t result = -1;
	NTSTATUS status;
	IO_STATUS_BLOCK I;
	FILE_ATTRIBUTE_TAG_INFORMATION INFO;

	status = NtQueryInformationFile(handle, &I, &INFO, sizeof(FILE_ATTRIBUTE_TAG_INFORMATION), FileAttributeTagInformation);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	if ((INFO.FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) == 0)
	{
		errno = EINVAL;
		return -1;
	}

	// Only support these two
	if (INFO.ReparseTag != IO_REPARSE_TAG_SYMLINK && INFO.ReparseTag != IO_REPARSE_TAG_MOUNT_POINT)
	{
		errno = EINVAL;
		return -1;
	}

	PREPARSE_DATA_BUFFER reparse_buffer = (PREPARSE_DATA_BUFFER)malloc(MAXIMUM_REPARSE_DATA_BUFFER_SIZE);
	status =
		NtFsControlFile(handle, NULL, NULL, NULL, &I, FSCTL_GET_REPARSE_POINT, NULL, 0, reparse_buffer, MAXIMUM_REPARSE_DATA_BUFFER_SIZE);

	if (INFO.ReparseTag == IO_REPARSE_TAG_SYMLINK)
	{
		if (reparse_buffer->SymbolicLinkReparseBuffer.SubstituteNameLength != 0)
		{
			wchar_t *data = (wchar_t *)malloc(reparse_buffer->SymbolicLinkReparseBuffer.SubstituteNameLength + 2);
			UNICODE_STRING u16_data;
			UTF8_STRING u8_data;
			memcpy(data,
				   reparse_buffer->SymbolicLinkReparseBuffer.PathBuffer +
					   reparse_buffer->SymbolicLinkReparseBuffer.SubstituteNameOffset / sizeof(WCHAR),
				   reparse_buffer->SymbolicLinkReparseBuffer.SubstituteNameLength);
			data[reparse_buffer->SymbolicLinkReparseBuffer.SubstituteNameLength / sizeof(WCHAR)] = L'\0';
			RtlInitUnicodeString(&u16_data, data);
			RtlUnicodeStringToUTF8String(&u8_data, &u16_data, TRUE);
			free(data);
			memcpy(buf, u8_data.Buffer, u8_data.Length <= bufsiz ? u8_data.Length : bufsiz);
			result = u8_data.Length <= bufsiz ? u8_data.Length : bufsiz;
			RtlFreeUTF8String(&u8_data);
		}
		else if (reparse_buffer->SymbolicLinkReparseBuffer.PrintNameLength != 0)
		{
			wchar_t *data = (wchar_t *)malloc(reparse_buffer->SymbolicLinkReparseBuffer.PrintNameLength + 2);
			UNICODE_STRING u16_data;
			UTF8_STRING u8_data;
			memcpy(data,
				   reparse_buffer->SymbolicLinkReparseBuffer.PathBuffer +
					   reparse_buffer->SymbolicLinkReparseBuffer.PrintNameOffset / sizeof(WCHAR),
				   reparse_buffer->SymbolicLinkReparseBuffer.PrintNameLength);
			data[reparse_buffer->SymbolicLinkReparseBuffer.PrintNameLength / sizeof(WCHAR)] = L'\0';
			RtlInitUnicodeString(&u16_data, data);
			RtlUnicodeStringToUTF8String(&u8_data, &u16_data, TRUE);
			free(data);
			memcpy(buf, u8_data.Buffer, u8_data.Length <= bufsiz ? u8_data.Length : bufsiz);
			result = u8_data.Length <= bufsiz ? u8_data.Length : bufsiz;
			RtlFreeUTF8String(&u8_data);
		}
		else
		{
			// This should not happen
			errno = EBADF;
		}
	}

	if (INFO.ReparseTag == IO_REPARSE_TAG_MOUNT_POINT)
	{
		if (reparse_buffer->MountPointReparseBuffer.SubstituteNameLength != 0)
		{
			wchar_t *data = (wchar_t *)malloc(reparse_buffer->MountPointReparseBuffer.SubstituteNameLength + 2);
			UNICODE_STRING u16_data;
			UTF8_STRING u8_data;
			memcpy(data,
				   reparse_buffer->MountPointReparseBuffer.PathBuffer +
					   reparse_buffer->MountPointReparseBuffer.SubstituteNameOffset / sizeof(WCHAR),
				   reparse_buffer->MountPointReparseBuffer.SubstituteNameLength);
			data[reparse_buffer->MountPointReparseBuffer.SubstituteNameLength / sizeof(WCHAR)] = L'\0';
			RtlInitUnicodeString(&u16_data, data);
			RtlUnicodeStringToUTF8String(&u8_data, &u16_data, TRUE);
			free(data);
			memcpy(buf, u8_data.Buffer, u8_data.Length <= bufsiz ? u8_data.Length : bufsiz);
			result = u8_data.Length <= bufsiz ? u8_data.Length : bufsiz;
			RtlFreeUTF8String(&u8_data);
		}
		else if (reparse_buffer->MountPointReparseBuffer.PrintNameLength != 0)
		{
			wchar_t *data = (wchar_t *)malloc(reparse_buffer->MountPointReparseBuffer.SubstituteNameLength + 2);
			UNICODE_STRING u16_data;
			UTF8_STRING u8_data;
			memcpy(data,
				   reparse_buffer->MountPointReparseBuffer.PathBuffer +
					   reparse_buffer->MountPointReparseBuffer.SubstituteNameOffset / sizeof(WCHAR),
				   reparse_buffer->MountPointReparseBuffer.SubstituteNameLength);
			data[reparse_buffer->MountPointReparseBuffer.SubstituteNameLength / sizeof(WCHAR)] = L'\0';
			RtlInitUnicodeString(&u16_data, data);
			RtlUnicodeStringToUTF8String(&u8_data, &u16_data, TRUE);
			free(data);
			memcpy(buf, u8_data.Buffer, u8_data.Length <= bufsiz ? u8_data.Length : bufsiz);
			result = u8_data.Length <= bufsiz ? u8_data.Length : bufsiz;
			RtlFreeUTF8String(&u8_data);
		}
		else
		{
			// This should not happen
			errno = EBADF;
		}
	}

	free(reparse_buffer);
	if (result != -1)
	{
		convert_bs_to_fs(buf, result);
	}
	return result;
}

ssize_t common_readlink(int dirfd, const char *restrict path, char *restrict buf, size_t bufsiz)
{
	wchar_t *u16_ntpath = get_absolute_ntpath(dirfd, path);
	if (u16_ntpath == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	HANDLE handle = just_open(u16_ntpath, FILE_READ_ATTRIBUTES, 0, FILE_OPEN, FILE_OPEN_REPARSE_POINT);
	free(u16_ntpath);

	if (handle == INVALID_HANDLE_VALUE)
	{
		// errno wil be set by just_open
		return -1;
	}

	int result = do_readlink(handle, buf, bufsiz);
	NtClose(handle);

	return result;
}

ssize_t wlibc_common_readlink(int dirfd, const char *restrict path, char *restrict buf, size_t bufsiz)
{
	if (buf == NULL)
	{
		errno = EINVAL;
		return -1;
	}

	if (path == NULL || path[0] == '\0')
	{
		if (validate_fd(dirfd) && (get_fd_flags(dirfd) & (O_NOFOLLOW | O_PATH)) == (O_NOFOLLOW | O_PATH))
		{
			return do_readlink(get_fd_handle(dirfd), buf, bufsiz);
		}

		errno = ENOENT;
		return -1;
	}

	VALIDATE_DIRFD(dirfd);

	return common_readlink(dirfd, path, buf, bufsiz);
}
