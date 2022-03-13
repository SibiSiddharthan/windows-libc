/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <unistd.h>
#include <stdlib.h>
#include <wchar.h>
#include <internal/nt.h>

char *wlibc_getcwd(char *buf, size_t size)
{
	char *return_buffer = NULL;
	UTF8_STRING u8_cwd;
	RtlUnicodeStringToUTF8String(&u8_cwd, &(NtCurrentPeb()->ProcessParameters->CurrentDirectory.DosPath), TRUE);
	// Dospath has a trailing slash remove it only if it is a directory. Ignore if at root of a volume.
	if (u8_cwd.Length != 3) // "C:\"
	{
		u8_cwd.Buffer[u8_cwd.Length - 1] = '\0';
	}

	if (size != 0 && size < u8_cwd.MaximumLength)
	{
		errno = ERANGE;
		goto finish;
	}

	if (buf == NULL)
	{
		if (size == 0)
		{
			return_buffer = (char *)malloc(u8_cwd.MaximumLength);
			memcpy(return_buffer, u8_cwd.Buffer, u8_cwd.MaximumLength);
		}
		else // size > u8_cwd.Length
		{
			return_buffer = (char *)malloc(size);
			memcpy(return_buffer, u8_cwd.Buffer, u8_cwd.MaximumLength);
		}
	}
	else
	{
		if (size == 0)
		{
			errno = ERANGE;
			goto finish;
		}
		return_buffer = buf;
		memcpy(buf, u8_cwd.Buffer, u8_cwd.MaximumLength);
	}

	for (int i = 0; i < u8_cwd.Length; i++)
	{
		if (return_buffer[i] == '\\')
		{
			return_buffer[i] = '/';
		}
	}

finish:
	RtlFreeUTF8String(&u8_cwd);
	return return_buffer;
}

wchar_t *wlibc_wgetcwd(wchar_t *wbuf, size_t length)
{
	PUNICODE_STRING cwd = &(NtCurrentPeb()->ProcessParameters->CurrentDirectory.DosPath);
	USHORT cwd_length = cwd->Length + sizeof(WCHAR);
	wchar_t *return_buffer = NULL;

	if (length != 0 && (length * sizeof(wchar_t)) < cwd_length)
	{
		errno = ERANGE;
		return NULL;
	}

	if (wbuf == NULL)
	{
		if (length == 0)
		{
			return_buffer = (wchar_t *)malloc(cwd_length);
			memcpy(return_buffer, cwd->Buffer, cwd_length);
		}
		else // length * sizeof(wchar_t) > cwd_length
		{
			return_buffer = (wchar_t *)malloc(sizeof(wchar_t) * length);
			memcpy(return_buffer, cwd->Buffer, cwd_length);
		}
	}
	else
	{
		if (length == 0)
		{
			errno = ERANGE;
			return NULL;
		}
		return_buffer = wbuf;
		memcpy(wbuf, cwd->Buffer, cwd_length);
	}

	if (cwd->Length != 6)
	{
		return_buffer[(cwd_length / sizeof(wchar_t)) - 2] = L'\0'; // L"C:\"
	}

	for (int i = 0; i < (int)(cwd_length / sizeof(wchar_t)) - 1; i++)
	{
		if (return_buffer[i] == L'\\')
		{
			return_buffer[i] = L'/';
		}
	}

	return return_buffer;
}
