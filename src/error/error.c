/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <error.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>

unsigned int error_message_count = 0;
static char program_name_buffer[260] = {0};

static char *get_program_name(void)
{
	int length = 0;
	int start = 0;
	UTF8_STRING u8_imagepath;
	RtlUnicodeStringToUTF8String(&u8_imagepath, &(NtCurrentPeb()->ProcessParameters->ImagePathName), TRUE);

	for (int i = u8_imagepath.Length;; --i)
	{
		if (u8_imagepath.Buffer[i - 1] == '\\')
		{
			start = i;
			break;
		}
		++length;
	}

	memcpy(program_name_buffer, u8_imagepath.Buffer + start, length - 4); // strip '.exe'
	RtlFreeUTF8String(&u8_imagepath);

	return program_name_buffer;
}

static void write_log(int errnum, const char *filename, unsigned int linenum, int do_strerror, const char *format, va_list args)
{
	int stderr_fileno, stdout_fileno;
	static char *program_name = NULL;

	if (program_name == NULL)
	{
		program_name = get_program_name();
	}

	stderr_fileno = fileno(stderr);
	if (stderr_fileno == -1)
	{
		return;
	}

	// Flush stdout if it is valid. This prevents interspersing of stdout and stderr
	// if they are writing to the same HANDLE
	stdout_fileno = fileno(stdout);
	if (stdout_fileno != -1)
	{
		wlibc_fflush(stdout);
	}

	if (filename)
	{
		// Eg "progname:file:line: error: strerror"
		fprintf(stderr, "%s:%s:%u: ", program_name, filename, linenum);
	}
	else
	{
		// Eg "progname: error: strerror"
		fprintf(stderr, "%s: ", program_name);
	}

	vfprintf(stderr, format, args);
	if (errnum && do_strerror)
	{
		fprintf(stderr, ": %s\n", strerror(errnum));
	}
	else
	{

		wlibc_fputc('\n', stderr);
	}

	wlibc_fflush(stderr);
	++error_message_count;
}

void wlibc_error(int status, int errnum, const char *filename, unsigned int linenum, int do_strerror, const char *format, va_list args)
{
	if (format == NULL)
	{
		errno = EINVAL;
		return;
	}

	write_log(errnum, filename, linenum, do_strerror, format, args);
	exit(status);
}

void wlibc_warn(int errnum, const char *filename, unsigned int linenum, int do_strerror, const char *format, va_list args)
{
	if (format == NULL)
	{
		errno = EINVAL;
		return;
	}

	write_log(errnum, filename, linenum, do_strerror, format, args);
}
