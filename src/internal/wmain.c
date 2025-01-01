/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/fcntl.h>
#include <internal/spawn.h>
#include <internal/security.h>
#include <internal/signal.h>
#include <internal/stdio.h>
#include <internal/thread.h>
#include <internal/timer.h>
#include <stdlib.h>

extern int main(int argc, char **argv, char **env);

// From errno/program.c
void init_program_name(void);

int exception_handler(DWORD code)
{
	switch (code)
	{
	// SIGSEGV
	case STATUS_ACCESS_VIOLATION:
	case STATUS_DATATYPE_MISALIGNMENT:
	case STATUS_ARRAY_BOUNDS_EXCEEDED:
	case STATUS_GUARD_PAGE_VIOLATION:
	case STATUS_IN_PAGE_ERROR:
		wlibc_raise(SIGSEGV);
		return EXCEPTION_CONTINUE_EXECUTION;

	// SIGFPE
	case STATUS_FLOAT_DENORMAL_OPERAND:
	case STATUS_FLOAT_DIVIDE_BY_ZERO:
	case STATUS_FLOAT_INEXACT_RESULT:
	case STATUS_FLOAT_INVALID_OPERATION:
	case STATUS_FLOAT_OVERFLOW:
	case STATUS_FLOAT_STACK_CHECK:
	case STATUS_FLOAT_UNDERFLOW:
	case STATUS_INTEGER_DIVIDE_BY_ZERO:
	case STATUS_INTEGER_OVERFLOW:
		wlibc_raise(SIGFPE);
		return EXCEPTION_CONTINUE_EXECUTION;

	// SIGILL
	case STATUS_ILLEGAL_INSTRUCTION:
	case STATUS_PRIVILEGED_INSTRUCTION:
	case STATUS_STACK_OVERFLOW:
		wlibc_raise(SIGILL);
		return EXCEPTION_CONTINUE_EXECUTION;

	default:
		return EXCEPTION_CONTINUE_SEARCH;
	}
}

int wmain(int argc, wchar_t **wargv, wchar_t **wenv)
{
	int envc = 0;
	char **argv = NULL, **env = NULL;
	UTF8_STRING *u8_args = NULL, *u8_env = NULL;

	if (argc)
	{
		argv = (char **)RtlAllocateHeap(NtCurrentProcessHeap(), 0, sizeof(char *) * (argc + 1)); // argv ends with NULL
		u8_args = (UTF8_STRING *)RtlAllocateHeap(NtCurrentProcessHeap(), 0, sizeof(UTF8_STRING) * argc);

		// Exit the process if we cannot create argv.
		if (argv == NULL || u8_args == NULL)
		{
			RtlExitUserProcess(STATUS_NO_MEMORY);
		}

		for (int i = 0; i < argc; i++)
		{
			NTSTATUS status;
			UNICODE_STRING u16_arg;
			RtlInitUnicodeString(&u16_arg, wargv[i]);
			status = RtlUnicodeStringToUTF8String(&u8_args[i], &u16_arg, TRUE);

			// Exit the process if we cannot create argv.
			if (status != STATUS_SUCCESS)
			{
				RtlExitUserProcess(status);
			}

			argv[i] = u8_args[i].Buffer;
		}
		argv[argc] = NULL;
	}

	if (wenv)
	{
		for (envc = 0; wenv[envc] != NULL; ++envc)
		{
			;
		}
		--envc;

		env = (char **)RtlAllocateHeap(NtCurrentProcessHeap(), 0, sizeof(char *) * (envc + 1)); // env ends with NULL
		u8_env = (UTF8_STRING *)RtlAllocateHeap(NtCurrentProcessHeap(), 0, sizeof(UTF8_STRING) * envc);

		// Exit the process if we cannot create env.
		if (env == NULL || u8_env == NULL)
		{
			RtlExitUserProcess(STATUS_NO_MEMORY);
		}

		for (int i = 0; i < envc; i++)
		{
			NTSTATUS status;
			UNICODE_STRING u16_env;
			RtlInitUnicodeString(&u16_env, wenv[i]);
			status = RtlUnicodeStringToUTF8String(&u8_env[i], &u16_env, TRUE);

			// Exit the process if we cannot create env.
			if (status != STATUS_SUCCESS)
			{
				RtlExitUserProcess(status);
			}

			env[i] = u8_env[i].Buffer;
		}
		env[envc] = NULL;
	}

#ifdef WLIBC_IO
	// DO NOT change the order of this.
	init_fd_table();
	initialize_stdio();
	atexit(cleanup_fd_table);
	atexit(cleanup_stdio);
#endif
#ifdef WLIBC_SIGNALS
	signal_init();
	atexit(signal_cleanup);
#endif
#ifdef WLIBC_SPAWN
	process_init();
	atexit(process_cleanup);
#endif
#ifdef WLIBC_THREADS
	threads_init();
	atexit(threads_cleanup);
#endif
#ifdef WLIBC_ACLS
	initialize_sids();
	atexit(cleanup_security_decsriptors);
#endif
#ifdef WLIBC_TIMERS
	initialize_itimers();
	atexit(cleanup_itimers);
#endif
#ifdef WLIBC_ERRNO
	init_program_name();
#endif

	int exit_status = 0;

	__try
	{
		exit_status = main(argc, argv, env);
	}
	__except (exception_handler(GetExceptionCode()))
	{
		;
	}

	if (argc)
	{
		for (int i = 0; i < argc; i++)
		{
			RtlFreeUTF8String(&u8_args[i]);
		}
		RtlFreeHeap(NtCurrentProcessHeap(), 0, u8_args);
		RtlFreeHeap(NtCurrentProcessHeap(), 0, argv);
	}

	if (wenv)
	{
		for (int i = 0; i < envc; i++)
		{
			RtlFreeUTF8String(&u8_env[i]);
		}
		RtlFreeHeap(NtCurrentProcessHeap(), 0, u8_env);
		RtlFreeHeap(NtCurrentProcessHeap(), 0, env);
	}

	return exit_status;
}
