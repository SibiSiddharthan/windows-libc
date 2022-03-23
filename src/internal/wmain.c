/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/fcntl.h>
#include <internal/process.h>
#include <internal/security.h>
#include <internal/signal.h>
#include <internal/stdio.h>
#include <internal/thread.h>
#include <stdlib.h>

extern int main(int argc, char **argv);

int wmain(int argc, wchar_t **wargv)
{
	char **argv = NULL;
	UTF8_STRING *u8_args = NULL;
	if (argc)
	{
		argv = (char **)malloc(sizeof(char *) * (argc + 1)); // argv ends with NULL
		u8_args = (UTF8_STRING *)malloc(sizeof(UTF8_STRING) * argc);
		for (int i = 0; i < argc; i++)
		{
			UNICODE_STRING u16_arg;
			RtlInitUnicodeString(&u16_arg, wargv[i]);
			RtlUnicodeStringToUTF8String(&u8_args[i], &u16_arg, TRUE);
			argv[i] = u8_args[i].Buffer;
		}
		argv[argc] = NULL;
	}

#ifdef WLIBC_POSIX_IO
	// DO NOT change the order of this.
	init_fd_table();
	initialize_stdio();
	atexit(cleanup_fd_table);
	atexit(cleanup_stdio);
#endif
#ifdef WLIBC_PROCESS
	process_init();
	atexit(process_cleanup);
#endif
#ifdef WLIBC_SIGNALS
	signal_init();
	atexit(signal_cleanup);
#endif
#ifdef WLIBC_THREADS
	threads_init();
	atexit(threads_cleanup);
#endif
#ifdef WLIBC_ACLS
	initialize_sids();
	atexit(cleanup_security_decsriptors);
#endif

	int exit_status = main(argc, argv);

	if (argc)
	{
		for (int i = 0; i < argc; i++)
		{
			RtlFreeUTF8String(&u8_args[i]);
		}
		free(argv);
	}

	return exit_status;
}
