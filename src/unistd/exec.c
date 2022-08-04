/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <errno.h>
#include <process.h>
#include <spawn.h>
#include <stdarg.h>
#include <sys/wait.h>

#define WLIBC_VALIDATE_SPAWN_MODE(mode)         \
	if (mode < _P_OVERLAY || mode > _P_NOWAITO) \
	{                                           \
		errno = EINVAL;                         \
		return -1;                              \
	}

int wlibc_execve(int use_path, const char *restrict path, char *restrict const argv[], char *restrict const env[])
{
	int status;
	pid_t pid;

	status = wlibc_common_spawn(&pid, path, NULL, NULL, use_path, argv, env);

	if (status == 0)
	{
		RtlExitUserProcess(0);
	}

	return status;
}

int wlibc_execl(int use_path, int env_given, const char *path, const char *arg0, va_list args)
{
	int argc = 0, envc = 0;
	va_list args_copy;

	va_copy(args_copy, args);

	if (arg0 != NULL)
	{
		++argc;
		while (va_arg(args, void *) != NULL)
		{
			++argc;
		}
	}

	if (env_given)
	{
		while (va_arg(args, void *) != NULL)
		{
			++envc;
		}
	}

	char **argv = NULL, **env = NULL;

	argv = (char **)RtlAllocateHeap(NtCurrentProcessHeap(), 0, sizeof(char *) * (argc + 1));
	env = (char **)RtlAllocateHeap(NtCurrentProcessHeap(), 0, sizeof(char *) * (envc + 1));

	if (argv == NULL || env == NULL)
	{
		errno = ENOMEM;
		return -1;
	}

	// argv
	for (int i = 0; i < argc; ++i)
	{
		if (i == 0)
		{
			argv[i] = (char *)arg0;
			continue;
		}

		argv[i] = va_arg(args_copy, char *);
	}

	argv[argc] = va_arg(args_copy, char *); // Should be NULL.

	// env
	if (env_given)
	{
		for (int i = 0; i < envc; ++i)
		{
			env[i] = va_arg(args_copy, char *);
		}

		env[envc] = va_arg(args_copy, char *); // Should be NULL.
	}

	va_end(args_copy);

	int status = wlibc_execve(use_path, path, argv, env);

	RtlFreeHeap(NtCurrentProcessHeap(), 0, argv);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, env);

	return status;
}

int wlibc_spawnve(int use_path, int mode, const char *restrict path, char *restrict const argv[], char *restrict const env[])
{
	// TODO P_DETACH
	int status;
	pid_t pid;

	WLIBC_VALIDATE_SPAWN_MODE(mode);

	status = wlibc_common_spawn(&pid, path, NULL, NULL, use_path, argv, env);

	if (status != 0)
	{
		return -1;
	}

	if (mode == _P_NOWAIT || mode == _P_DETACH)
	{
		return pid;
	}
	else if (mode == _P_WAIT)
	{
		// We have just added a child and are waiting for it. This call will not fail.
		waitpid(pid, &status, 0);
		return status;
	}
	else // (mode == _P_OVERLAY)
	{
		RtlExitUserProcess(0);
	}
}

int wlibc_spawnl(int use_path, int env_given, int mode, const char *path, const char *arg0, va_list args)
{
	int argc = 0, envc = 0;
	va_list args_copy;

	va_copy(args_copy, args);

	if (arg0 != NULL)
	{
		++argc;
		while (va_arg(args, void *) != NULL)
		{
			++argc;
		}
	}

	if (env_given)
	{
		while (va_arg(args, void *) != NULL)
		{
			++envc;
		}
	}

	char **argv = NULL, **env = NULL;

	argv = (char **)RtlAllocateHeap(NtCurrentProcessHeap(), 0, sizeof(char *) * (argc + 1));
	env = (char **)RtlAllocateHeap(NtCurrentProcessHeap(), 0, sizeof(char *) * (envc + 1));

	if (argv == NULL || env == NULL)
	{
		errno = ENOMEM;
		return -1;
	}

	// argv
	for (int i = 0; i < argc; ++i)
	{
		if (i == 0)
		{
			argv[i] = (char *)arg0;
			continue;
		}

		argv[i] = va_arg(args_copy, char *);
	}

	argv[argc] = va_arg(args_copy, char *); // Should be NULL.

	// env
	if (env_given)
	{
		for (int i = 0; i < envc; ++i)
		{
			env[i] = va_arg(args_copy, char *);
		}

		env[envc] = va_arg(args_copy, char *); // Should be NULL.
	}

	va_end(args_copy);

	int status = wlibc_spawnve(use_path, mode, path, argv, env);

	RtlFreeHeap(NtCurrentProcessHeap(), 0, argv);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, env);

	return status;
}
