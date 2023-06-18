/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <corecrt_stdio_config.h>

int __cdecl __stdio_common_vsprintf_p(_In_ unsigned __int64 _Options, _Out_writes_z_(_BufferCount) char *_Buffer, _In_ size_t _BufferCount,
									  _In_z_ _Printf_format_string_params_(2) char const *_Format, _In_opt_ _locale_t _Locale,
									  va_list _ArgList);

int number_of_chars(const char *restrict format, va_list args)
{
	if (format == NULL)
	{
		errno = EINVAL;
		return -1;
	}

	return __stdio_common_vsprintf_p(_CRT_INTERNAL_LOCAL_PRINTF_OPTIONS | _CRT_INTERNAL_PRINTF_STANDARD_SNPRINTF_BEHAVIOR, NULL, 0, format,
									 NULL, args);
}

int print_chars(char *restrict buffer, size_t size, const char *restrict format, va_list args)
{
	return __stdio_common_vsprintf_p(_CRT_INTERNAL_LOCAL_PRINTF_OPTIONS | _CRT_INTERNAL_PRINTF_STANDARD_SNPRINTF_BEHAVIOR, buffer, size,
									 format, NULL, args);
}

int wlibc_vfprintf(FILE *restrict stream, const char *restrict format, va_list args)
{
	int count = number_of_chars(format, args);
	if (count == -1)
	{
		return -1;
	}

	char *buffer = (char *)RtlAllocateHeap(NtCurrentProcessHeap(), 0, count + 1);
	if (buffer == NULL)
	{
		errno = ENOMEM;
		return -1;
	}

	print_chars(buffer, count + 1, format, args);
	int result = (int)fwrite(buffer, 1, count, stream);

	RtlFreeHeap(NtCurrentProcessHeap(), 0, buffer);
	return result;
}

int wlibc_vdprintf(int fd, const char *restrict format, va_list args)
{
	int count = number_of_chars(format, args);
	if (count == -1)
	{
		return -1;
	}

	char *buffer = (char *)RtlAllocateHeap(NtCurrentProcessHeap(), 0, count + 1);
	if (buffer == NULL)
	{
		errno = ENOMEM;
		return -1;
	}

	print_chars(buffer, count + 1, format, args);
	int result = (int)write(fd, buffer, count);

	RtlFreeHeap(NtCurrentProcessHeap(), 0, buffer);
	return result;
}

int wlibc_vasprintf(char **restrict buffer, const char *restrict format, va_list args)
{
	int count = number_of_chars(format, args);
	if (count == -1)
	{
		return -1;
	}

	// User freeable buffer, use malloc.
	*buffer = (char *)malloc(count + 1);
	if (*buffer == NULL)
	{
		errno = ENOMEM;
		return -1;
	}

	return print_chars(*buffer, count + 1, format, args);
}

int wlibc_vsnprintf(char *restrict buffer, size_t size, const char *restrict format, va_list args)
{
	if (format == NULL)
	{
		errno = EINVAL;
		return -1;
	}
	if (buffer != NULL && size == 0)
	{
		errno = EINVAL;
		return -1;
	}

	return __stdio_common_vsprintf_p(_CRT_INTERNAL_LOCAL_PRINTF_OPTIONS | _CRT_INTERNAL_PRINTF_STANDARD_SNPRINTF_BEHAVIOR, buffer, size,
									 format, NULL, args);
}

char *wlibc_vasnprintf(char *restrict buffer, size_t *size, const char *restrict format, va_list args)
{
	int count = number_of_chars(format, args);
	if (count == -1)
	{
		return NULL;
	}

	// Use the buffer provided.
	if (buffer != NULL && *size >= (size_t)count)
	{
		count = print_chars(buffer, *size, format, args);
		*size = count;
		return buffer;
	}
	else
	{
		// Allocate a new buffer.
		buffer = (char *)malloc(count + 1);
		if (buffer == NULL)
		{
			errno = ENOMEM;
			return NULL;
		}

		count = print_chars(buffer, *size, format, args);
		*size = count;
		return buffer;
	}
}
