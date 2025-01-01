/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <corecrt_stdio_config.h>

int __cdecl __stdio_common_vsscanf(_In_ unsigned __int64 _Options, _In_reads_(_BufferCount) _Pre_z_ char const *_Buffer,
								   _In_ size_t _BufferCount, _In_z_ _Scanf_format_string_params_(2) char const *_Format,
								   _In_opt_ _locale_t _Locale, va_list _ArgList);

int wlibc_vfscanf(FILE *restrict stream, const char *restrict format, va_list args)
{
	if (format == NULL)
	{
		errno = EINVAL;
		return -1;
	}

	if(stream == NULL)
	{
		return -1;
	}

#if 0
	ssize_t initial_pos = wlibc_ftell(stream);
	// Assume no one is going to read more than 1024 characters at a time.
	// This is a BAD assumption and will only work once. Let's keep it for the time being
	char buffer[1024];
	memset(buffer,0,1024);
	fread(buffer,1,1024,stream);
#endif
	char buffer[1024];
	int result = wlibc_vsscanf(buffer, format, args);

	return result;
	// TODO this is hard
}

int wlibc_vsscanf(const char *restrict str, const char *restrict format, va_list args)
{
	if (format == NULL || str == NULL)
	{
		errno = EINVAL;
		return -1;
	}
	return __stdio_common_vsscanf(_CRT_INTERNAL_LOCAL_SCANF_OPTIONS, str, (size_t)-1, format, NULL, args);
}
