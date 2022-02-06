/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <errno.h>
#include <unistd.h>
#include <internal/error.h>
#include <Windows.h>
#include <fcntl.h>
#include <internal/process.h>
#include <stdarg.h>
#include <process.h>
#include <internal/misc.h>

int common_spawn(int mode, int search_path, const wchar_t *file, wchar_t **argv, wchar_t **env)
{
	wchar_t *args = NULL, *envp = NULL;
	size_t args_size = 0, env_size = 0;

	wchar_t **args_temp = argv;
	while (*argv)
	{
		args_size += wcslen(*argv) + 1;
		++argv;
	}
	argv = args_temp;

	args = (wchar_t *)malloc(sizeof(wchar_t) * args_size);
	while (*argv)
	{
		wcscat(args, *argv);
		wcscat(args, L" ");
		++argv;
	}
	argv = args_temp;

	if (env)
	{
		wchar_t **env_temp = env;
		while (*env)
		{
			env_size += wcslen(*env) + 1;
			++env;
		}
		env = env_temp;

		envp = (wchar_t *)malloc(sizeof(wchar_t) * (env_size + 1)); // env should end with 2 NULLs
		while (*env)
		{
			wcscat(envp, *env);
			wcscat(envp, L"\0");
			++env;
		}
		env = env_temp;
		envp[env_size] = L'\0';
	}

	DWORD flags = CREATE_UNICODE_ENVIRONMENT;
	if (mode == _P_DETACH)
	{
		flags |= DETACHED_PROCESS;
	}

	STARTUPINFO SINFO;
	PROCESS_INFORMATION PINFO;
	memset(&SINFO, 0, sizeof(SINFO));
	SINFO.cb = sizeof(SINFO);
	memset(&PINFO, 0, sizeof(PINFO));

	BOOL status = CreateProcess(file, args, NULL, NULL, TRUE, flags, envp, NULL, &SINFO, &PINFO);
	if (status == 0)
	{
		DWORD error = GetLastError();
		if (error != ERROR_BAD_EXE_FORMAT)
		{
			map_win32_error_to_wlibc(error);
			free(args);
			if (env)
			{
				free(envp);
			}
			return -1;
		}

		// shebang spawn TODO
	}

	free(args);
	if (env)
	{
		free(envp);
	}

	if (mode == _P_OVERLAY)
	{
		// do exec
		// TODO make sure our initializations are cleaned up
		exit(0);
	}

	if (mode == _P_WAIT)
	{
		WaitForSingleObject(PINFO.hProcess, INFINITE);
		DWORD exit_code;
		if (!GetExitCodeProcess(PINFO.hProcess, &exit_code))
		{
			map_win32_error_to_wlibc(GetLastError());
			return -1;
		}

		return exit_code;
	}

	if (mode == _P_DETACH || mode == _P_NOWAIT)
	{
		add_child(PINFO.dwProcessId, PINFO.hProcess);
		return PINFO.dwProcessId;
	}

	// unreachable
	return -1;
}

int wlibc_common_spawnl(int mode, int search_path, int argc, int envc, const char *file, const char *arg0, va_list args)
{
	if (stricmp(file, arg0) != 0)
	{
		errno = EINVAL;
		return -1;
	}

	wchar_t **wargv = NULL, **wenv = NULL;

	wargv = (wchar_t **)malloc(sizeof(wchar_t *) * (argc + 2)); // Terminating null + arg0
	wargv[0] = mb_to_wc(arg0);
	for (int i = 0; i < argc; i++)
	{
		wargv[i + 1] = mb_to_wc(va_arg(args, char *));
	}
	wargv[argc + 1] = NULL;

	if (envc)
	{
		va_arg(args, char *); // Move past the terminatinfg NULL for argv
		wenv = (wchar_t **)malloc(sizeof(wchar_t *) * (envc + 1));
		for (int i = 0; i < envc; i++)
		{
			wenv[i + 1] = mb_to_wc(va_arg(args, char *));
		}
		wenv[envc] = NULL;
	}

	wchar_t *wfile = mb_to_wc(file);

	int status = common_spawn(mode, search_path, wfile, wargv, wenv);

	free(wfile);

	// free wargv
	for (int i = 0; i < argc + 1; i++)
	{
		free(wargv[i]);
	}
	free(wargv);

	// free wenv
	if (envc)
	{
		for (int i = 0; i < envc; i++)
		{
			free(wenv[i]);
		}
		free(wenv);
	}

	return status;
}

int wlibc_common_spawnv(int mode, int search_path, const char *file, char const *const *argv, char const *const *env)
{
	if (argv[0] == NULL || stricmp(file, argv[0]) != 0)
	{
		errno = EINVAL;
		return -1;
	}

	wchar_t *wfile = mb_to_wc(file);
	wchar_t **wargv = NULL, **wenv = NULL;
	int argv_count = 0, env_count = 0;

	char const *const *argv_temp = argv;
	while (*(argv++))
	{
		++argv_count;
	}
	argv = argv_temp;

	wargv = (wchar_t **)malloc(sizeof(wchar_t *) * (argv_count + 1)); // Include the terminating null as well
	for (int i = 0; i < argv_count; i++)
	{
		wargv[i] = mb_to_wc(argv[i]);
	}
	wargv[argv_count] = NULL;

	if (env)
	{
		char const *const *env_temp = env;
		while (*(env++))
		{
			++env_count;
		}
		env = env_temp;

		wenv = (wchar_t **)malloc(sizeof(wchar_t *) * (env_count + 1)); // Include the terminating null as well
		for (int i = 0; i < env_count; i++)
		{
			wenv[i] = mb_to_wc(argv[i]);
		}
		wenv[env_count] = NULL;
	}

	int status = common_spawn(mode, search_path, wfile, wargv, wenv);

	free(wfile);

	// free wargv
	for (int i = 0; i < argv_count; i++)
	{
		free(wargv[i]);
	}
	free(wargv);

	// free wenv
	if (env)
	{
		for (int i = 0; i < env_count; i++)
		{
			free(wenv[i]);
		}
		free(wenv);
	}

	return status;
}

int wlibc_common_wspawnl(int mode, int search_path, int argc, int envc, const wchar_t *file, const wchar_t *arg0, va_list args)
{
	if (wcsicmp(file, arg0) != 0)
	{
		errno = EINVAL;
		return -1;
	}

	wchar_t **argv = NULL, **env = NULL;

	argv = (wchar_t **)malloc(sizeof(wchar_t *) * (argc + 2)); // Terminating null + arg0
	argv[0] = (wchar_t*)arg0;
	for (int i = 0; i < argc; i++)
	{
		argv[i + 1] = va_arg(args, wchar_t *);
	}
	argv[argc + 1] = NULL;

	if (envc)
	{
		va_arg(args, wchar_t *); // Move past the terminatinfg NULL for argv
		env = (wchar_t **)malloc(sizeof(wchar_t *) * (envc + 1));
		for (int i = 0; i < envc; i++)
		{
			env[i + 1] = va_arg(args, wchar_t *);
		}
		env[envc] = NULL;
	}

	int status = common_spawn(mode, search_path, file, argv, env);

	free(argv);
	if (envc)
	{
		free(env);
	}

	return status;
}

int wlibc_common_wspawnv(int mode, int search_path, const wchar_t *file, wchar_t const *const *argv, wchar_t const *const *env)
{
	if (argv[0] == NULL || wcsicmp(file, argv[0]) != 0)
	{
		errno = EINVAL;
		return -1;
	}

	return common_spawn(mode, search_path, file, (wchar_t**)argv, (wchar_t**)env);
}
