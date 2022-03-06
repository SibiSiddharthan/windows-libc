/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <internal/fcntl.h>
#include <internal/path.h>
#include <internal/process.h>
#include <errno.h>
#include <fcntl.h>
#include <spawn.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

int do_open(int dirfd, const char *name, int oflags, mode_t perm);

typedef struct _inherit_fdinfo
{
	HANDLE handle;
	handle_t type;
	int flags;
} inherit_fdinfo;

typedef struct _inherit_information
{
	int fds;
	inherit_fdinfo *fdinfo;
} inherit_information;

static void initialize_inherit_information(inherit_information *info, int max_fd)
{
	info->fdinfo = (inherit_fdinfo *)malloc(sizeof(inherit_fdinfo) * (max_fd + 1));
	info->fds = max_fd;

	EnterCriticalSection(&_fd_critical);
	for (int i = 0; i <= max_fd; ++i)
	{
		if (i < (int)_fd_table_size)
		{
			// This can be memcpy'd. TODO
			info->fdinfo[i].handle = _fd_io[i]._handle;
			info->fdinfo[i].type = _fd_io[i]._type;
			info->fdinfo[i].flags = _fd_io[i]._flags;
		}
		else
		{
			info->fdinfo[i].handle = INVALID_HANDLE_VALUE;
			info->fdinfo[i].type = 0;
			info->fdinfo[i].flags = 0;
		}
	}
	LeaveCriticalSection(&_fd_critical);
}

static void cleanup_inherit_information(inherit_information *restrict info, const spawn_actions_t *restrict actions)
{
	if (info->fdinfo == NULL)
	{
		return;
	}

	if (actions)
	{
		for (int i = 0; i < actions->used; ++i)
		{
			if (actions->actions[i].type == open_action)
			{
				// Close the handle that was duplicated.
				int fd = actions->actions[i].open_action.fd;
				HANDLE handle = info->fdinfo[fd].handle;
				NtClose(handle);
			}
		}
	}

	free(info->fdinfo);
}

#define FOPEN_FLAG      0x01 // file handle open
#define FEOFLAG_FLAG    0x02 // end of file has been encountered
#define FCRLF_FLAG      0x04 // CR-LF across read buffer (in text mode)
#define FPIPE_FLAG      0x08 // file handle refers to a pipe
#define FNOINHERIT_FLAG 0x10 // file handle opened _O_NOINHERIT
#define FAPPEND_FLAG    0x20 // file handle opened O_APPEND
#define FNULL_FLAG      0x40 // file handle refers to a device (Originally FDEV)
#define FCONSOLE_FLAG   0x80 // file handle is in text mode  (Originally FTEXT)

static void give_inherit_information_to_startupinfo(inherit_information *inherit_info, STARTUPINFOW *startup_info)
{
	// First count the maximum fd that is to be inherited.
	int real_max_fd = 0;

	for (int i = 0; i <= inherit_info->fds; ++i)
	{
		if (inherit_info->fdinfo[i].handle != INVALID_HANDLE_VALUE)
		{
			real_max_fd = i;
		}
	}

	startup_info->dwFlags |= STARTF_USESTDHANDLES;

	// First 4 bytes denote number of handles inherited (say N).      4
	// Then for each handle 1 byte is use to denote the flag values.  N
	// Then each of the handles are listed as DWORDs.                4N
	startup_info->cbReserved2 = sizeof(DWORD) + 5 * (real_max_fd + 1);
	startup_info->lpReserved2 = malloc(startup_info->cbReserved2);

	*(DWORD *)startup_info->lpReserved2 = real_max_fd;
	off_t handle_start = 4 + real_max_fd + 1;

	for (int i = 0; i <= real_max_fd; ++i)
	{
		if (i > 2)
		{
			unsigned char flag = FOPEN_FLAG;

			if (inherit_info->fdinfo[i].flags & O_APPEND)
			{
				flag |= FAPPEND_FLAG;
			}

			if (inherit_info->fdinfo[i].type == PIPE_HANDLE)
			{
				flag |= FPIPE_FLAG;
			}
			else if (inherit_info->fdinfo[i].type == CONSOLE_HANDLE)
			{
				flag |= FCONSOLE_FLAG;
			}
			else if (inherit_info->fdinfo[i].type == NULL_HANDLE)
			{
				flag |= FNULL_FLAG;
			}

			((UCHAR *)(startup_info->lpReserved2 + 4))[i] = flag;
			((DWORD *)(startup_info->lpReserved2 + handle_start))[i] = (DWORD)(LONG_PTR)inherit_info->fdinfo[i].handle;
		}
		// Always give the std handles in startupinfo itself
		// Mark the handles as invalid in the lpReserved2.
		else
		{
			switch (i)
			{
			// Instead of marking these handles as invalid we can use this to store sigset, sigdefault and one more thing.
			// Mark the flag as not open as well, so that these values will be treated as invalid by the
			// CRT's lowio initialization routine.
			case 0:
				startup_info->hStdInput = inherit_info->fdinfo[0].handle;
				((DWORD *)(startup_info->lpReserved2 + handle_start))[0] = -1;
				((UCHAR *)(startup_info->lpReserved2 + 4))[0] = 0;
				break;
			case 1:
				startup_info->hStdOutput = inherit_info->fdinfo[1].handle;
				((DWORD *)(startup_info->lpReserved2 + handle_start))[1] = -1;
				((UCHAR *)(startup_info->lpReserved2 + 4))[1] = 0;
				break;
			case 2:
				startup_info->hStdError = inherit_info->fdinfo[2].handle;
				((DWORD *)(startup_info->lpReserved2 + handle_start))[2] = -1;
				((UCHAR *)(startup_info->lpReserved2 + 4))[2] = 0;
				break;
			}
		}
	}
}

static WCHAR *convert_argv_to_wargv(char *const argv[])
{
	UNICODE_STRING *u16_args = NULL;
	UTF8_STRING *u8_args = NULL;
	WCHAR *wargv = NULL;

	int argc = 0;
	size_t argv_size = 0;
	size_t argv_used = 0;
	char *const *pargv = argv;

	// Count the number of elements in argv.
	while (*argv)
	{
		++argc;
		++argv;
	}
	argv = pargv;

	u8_args = (UTF8_STRING *)malloc(sizeof(UTF8_STRING) * argc);
	u16_args = (UNICODE_STRING *)malloc(sizeof(UNICODE_STRING) * argc);

	for (int i = 0; i < argc; ++i)
	{
		RtlInitUTF8String(&u8_args[i], argv[i]);
		argv_size += u8_args[i].MaximumLength;
	}

	argv_size *= sizeof(WCHAR);
	wargv = (WCHAR *)malloc(argv_size);

	for (int i = 0; i < argc; ++i)
	{
		u16_args[i].Buffer = (WCHAR *)((char *)wargv + argv_used);
		u16_args[i].Length = argv_size - argv_used;
		u16_args[i].MaximumLength = u16_args[i].Length;

		RtlUTF8StringToUnicodeString(&u16_args[i], &u8_args[i], FALSE);

		// In Windows program arguments are separated by a ' '.
		// Change the terminating NULL to a L' '.
		wargv[u16_args[i].Length / sizeof(WCHAR)] = L' ';
		argv_used += u16_args[i].Length + 1;
	}

	// Finally put in the terminating NULL
	wargv[(argv_used - 1) / sizeof(WCHAR)] = L'\0';

	free(u16_args);
	free(u8_args);

	return wargv;
}

static WCHAR *convert_env_to_wenv(char *const env[])
{
	UNICODE_STRING *u16_envs = NULL;
	UTF8_STRING *u8_envs = NULL;
	WCHAR *wenv = NULL;

	int envc = 0;
	size_t env_size = 0;
	size_t env_used = 0;
	char *const *penv = env;

	// Count the number of elements in env.
	while (*env)
	{
		++envc;
		++env;
	}
	env = penv;

	u8_envs = (UTF8_STRING *)malloc(sizeof(UTF8_STRING) * envc);
	u16_envs = (UNICODE_STRING *)malloc(sizeof(UNICODE_STRING) * envc);

	for (int i = 0; i < envc; ++i)
	{
		RtlInitUTF8String(&u8_envs[i], env[i]);
		env_size += u8_envs[i].MaximumLength;
	}

	++env_size; // For the terminating 2 NULLs.
	env_size *= sizeof(WCHAR);
	wenv = (WCHAR *)malloc(env_size);

	for (int i = 0; i < envc; ++i)
	{
		u16_envs[i].Buffer = (WCHAR *)((char *)wenv + env_used);
		u16_envs[i].Length = env_size - env_used;
		u16_envs[i].MaximumLength = u16_envs[i].Length;

		RtlUTF8StringToUnicodeString(&u16_envs[i], &u8_envs[i], FALSE);

		env_used += u16_envs[i].Length + 1;
	}

	// Finally put in the terminating NULL.
	// NOTE: Since 'RtlUTF8StringToUnicodeString' appends a terminating NULL to the converted string,
	// we only need to put the other terminating NULL to denote the end of the environment.
	wenv[env_used / sizeof(WCHAR)] = L'\0';

	free(u16_envs);
	free(u8_envs);

	return wenv;
}

int wlibc_spawn(pid_t *restrict pid, const char *restrict path, const spawn_actions_t *restrict actions,
				const spawnattr_t *restrict attributes, int use_path, char *restrict const argv[], char *restrict const env[])
{
	// TODO
	// Ignore spawn attributes for now.
	// Ignore using path for now.
	int result = -1;

	STARTUPINFOW SINFO;
	PROCESS_INFORMATION PINFO;
	UNICODE_STRING u16_path, *pu16_cwd = NULL;
	UTF8_STRING u8_path;
	WCHAR *wpath = NULL;
	WCHAR *wargv = NULL;
	WCHAR *wenv = NULL;
	WCHAR *wcwd = NULL;

	memset(&SINFO, 0, sizeof(SINFO));
	SINFO.cb = sizeof(SINFO);
	memset(&PINFO, 0, sizeof(PINFO));

	// Convert path to UTF-16
	RtlInitUTF8String(&u8_path, path);
	RtlUTF8StringToUnicodeString(&u16_path, &u8_path, TRUE);
	wpath = u16_path.Buffer;

	// Convert args to UTF-16
	wargv = convert_argv_to_wargv((char *const *)argv);

	// Convert env to UTF-16
	if (env)
	{
		wenv = convert_env_to_wenv((char *const *)env);
	}

	// Perform the actions.
	int num_of_open_actions = 0;
	int num_of_close_actions = 0;
	int num_of_dup2_actions = 0;
	int num_of_chdir_actions = 0;
	int num_of_fchdir_actions = 0;
	int max_fd_requested = 0;

	for (int i = 0; i < actions->used; ++i)
	{
		switch (actions->actions[i].type)
		{
		case open_action:
			++num_of_open_actions;
			max_fd_requested = __max(actions->actions[i].open_action.fd, max_fd_requested);
			break;
		case close_action:
			++num_of_close_actions;
			break;
		case dup2_action:
			++num_of_dup2_actions;
			max_fd_requested = __max(actions->actions[i].dup2_action.newfd, max_fd_requested);
			break;
		case chdir_action:
			++num_of_chdir_actions;
		case fchdir_action:
			++num_of_fchdir_actions;
			break;
		}

		if (num_of_chdir_actions + num_of_fchdir_actions > 1)
		{
			// More than directory change requested.
			errno = EINVAL;
			goto finish;
		}
	}

	max_fd_requested = __max(max_fd_requested, (int)_fd_table_size);
	inherit_information inherit_info;

	initialize_inherit_information(&inherit_info, max_fd_requested);

	NTSTATUS ntstatus;

	for (int i = 0; i < actions->used; ++i)
	{
		switch (actions->actions[i].type)
		{

		case open_action:
		{
			int fd = do_open(AT_FDCWD, actions->actions[i].open_action.path, actions->actions[i].open_action.oflag,
							 actions->actions[i].open_action.mode);
			if (fd < 0)
			{
				// errno wil be set by `do_open`.
				goto finish;
			}

			int flags = get_fd_flags(fd);
			handle_t type = get_fd_type(fd);
			HANDLE handle = get_fd_handle(fd);
			HANDLE nhandle;
			// Duplicate this fd and make sure it is inheritable.
			ntstatus = NtDuplicateObject(NtCurrentProcess(), handle, NtCurrentProcess(), &nhandle, 0, OBJ_CASE_INSENSITIVE | OBJ_INHERIT,
										 DUPLICATE_SAME_ACCESS);
			if (ntstatus != STATUS_SUCCESS)
			{
				map_ntstatus_to_errno(ntstatus);
				goto finish;
			}

			int nfd = actions->actions[i].open_action.fd;
			// Populate the inherit info.
			inherit_info.fdinfo[nfd].handle = nhandle;
			inherit_info.fdinfo[nfd].flags = flags;
			inherit_info.fdinfo[nfd].type = type;

			// Internal close function should not fail.
			close_fd(fd);
		}
		break;

		case close_action:
		{
			int fd = actions->actions[i].close_action.fd;
			OBJECT_HANDLE_FLAG_INFORMATION flag_info;

			if (!validate_fd(fd))
			{
				errno = EBADF;
				goto finish;
			}

			int flags = get_fd_flags(fd);
			HANDLE handle = get_fd_handle(fd);

			//  Make sure that this handle is not inheritable, if not make it so.
			if ((flags & O_NOINHERIT) == 0)
			{
				flag_info.Inherit = FALSE;
				flag_info.ProtectFromClose = FALSE;

				ntstatus = NtSetInformationObject(handle, ObjectHandleFlagInformation, &flag_info, sizeof(OBJECT_HANDLE_FLAG_INFORMATION));
				if (ntstatus != STATUS_SUCCESS)
				{
					map_ntstatus_to_errno(ntstatus);
					goto finish;
				}
			}

			inherit_info.fdinfo[fd].handle = INVALID_HANDLE_VALUE;
			inherit_info.fdinfo[fd].flags = 0;
			inherit_info.fdinfo[fd].type = 0;
		}
		break;

		case dup2_action:
		{
			int oldfd = actions->actions[i].dup2_action.oldfd;
			int newfd = actions->actions[i].dup2_action.newfd;

			OBJECT_HANDLE_FLAG_INFORMATION flag_info;
			if (!validate_fd(oldfd))
			{
				errno = EBADF;
				goto finish;
			}

			int flags = get_fd_flags(oldfd);
			HANDLE handle = get_fd_handle(oldfd);
			handle_t type = get_fd_type(oldfd);

			//  Make sure that this handle is inheritable, if not make it so.
			if (flags & O_NOINHERIT)
			{
				flag_info.Inherit = TRUE;
				flag_info.ProtectFromClose = FALSE;

				ntstatus = NtSetInformationObject(handle, ObjectHandleFlagInformation, &flag_info, sizeof(OBJECT_HANDLE_FLAG_INFORMATION));
				if (ntstatus != STATUS_SUCCESS)
				{
					map_ntstatus_to_errno(ntstatus);
					goto finish;
				}
			}

			inherit_info.fdinfo[newfd].handle = handle;
			inherit_info.fdinfo[newfd].flags = flags & ~O_NOINHERIT;
			inherit_info.fdinfo[newfd].type = type;
		}
		break;

		case chdir_action:
		{
			// Validate the chdir path given.
			pu16_cwd = xget_absolute_dospath(AT_FDCWD, actions->actions[i].chdir_action.path);
			if (pu16_cwd == NULL)
			{
				errno = ENOENT;
				goto finish;
			}
			wenv = pu16_cwd->Buffer;
		}
		break;

		case fchdir_action:
		{
			pu16_cwd = xget_fd_dospath(actions->actions[i].fchdir_action.fd);
			if (pu16_cwd == NULL)
			{
				errno = EBADF;
				goto finish;
			}
			wenv = pu16_cwd->Buffer;
		}
		break;
		}
	}

	give_inherit_information_to_startupinfo(&inherit_info, &SINFO);
	// Do the spawn.

	BOOL status = CreateProcessW(wpath, wargv, NULL, NULL, TRUE, CREATE_UNICODE_ENVIRONMENT, wenv, wcwd, &SINFO, &PINFO);
	if (status == 0)
	{
		DWORD error = GetLastError();
		if (error != ERROR_BAD_EXE_FORMAT)
		{
			map_doserror_to_errno(error);
			return -1;
		}

		// shebang spawn TODO
	}

	// Add the child information to our process table.
	add_child(PINFO.dwProcessId, PINFO.hProcess);
	*pid = PINFO.dwProcessId;
	result = 0;

finish:
	RtlFreeUnicodeString(&u16_path);
	free(wargv);
	free(wenv);
	free(pu16_cwd);
	free(SINFO.lpReserved2);
	cleanup_inherit_information(&inherit_info, actions);

	return result;
}
