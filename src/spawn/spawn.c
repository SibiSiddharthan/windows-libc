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

	SHARED_LOCK_FD_TABLE();
	for (int i = 0; i <= max_fd; ++i)
	{
		if (i < (int)_wlibc_fd_table_size)
		{
			// This can be memcpy'd. TODO
			info->fdinfo[i].handle = _wlibc_fd_table[i].handle;
			info->fdinfo[i].type = _wlibc_fd_table[i].type;
			info->fdinfo[i].flags = _wlibc_fd_table[i].flags;
		}
		else
		{
			info->fdinfo[i].handle = INVALID_HANDLE_VALUE;
			info->fdinfo[i].type = 0;
			info->fdinfo[i].flags = 0;
		}
	}
	SHARED_UNLOCK_FD_TABLE();
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

// osfile values from corecrt_internal_lowio.h.
#define FOPEN_FLAG      0x01 // file handle open
#define FEOFLAG_FLAG    0x02 // end of file has been encountered
#define FCRLF_FLAG      0x04 // CR-LF across read buffer (in text mode)
#define FPIPE_FLAG      0x08 // file handle refers to a pipe
#define FNOINHERIT_FLAG 0x10 // file handle opened O_NOINHERIT
#define FAPPEND_FLAG    0x20 // file handle opened O_APPEND
#define FDEV_FLAG       0x40 // file handle refers to a device
#define FTEXT_FLAG      0x80 // file handle is in text mode

// Processes spawned by wlibc should be compatible with the initialization routine of msvcrt.
static void give_inherit_information_to_startupinfo(inherit_information *inherit_info, STARTUPINFOW *startup_info)
{
	// First count the maximum fd that is to be inherited.
	int number_of_fds_to_inherit = 0;
	int real_max_fd = 0;

	for (int i = 0; i <= inherit_info->fds; ++i)
	{
		if (inherit_info->fdinfo[i].handle != INVALID_HANDLE_VALUE)
		{
			real_max_fd = i;
		}
	}

	number_of_fds_to_inherit = real_max_fd + 1;

	startup_info->dwFlags |= STARTF_USESTDHANDLES;

	// First 4 bytes denote number of handles inherited (say N).      4
	// Then for each handle 1 byte is use to denote the flag values.  N
	// Then each of the handles are listed.                          8N
	// NOTE: The documentation in lowio/ioinit.cpp says the HANDLES are passed as DWORDS.
	// This is incorrect.
	startup_info->cbReserved2 = (WORD)(sizeof(DWORD) + (sizeof(HANDLE) + sizeof(UCHAR)) * number_of_fds_to_inherit);
	startup_info->lpReserved2 = malloc(startup_info->cbReserved2);

	*(DWORD *)startup_info->lpReserved2 = number_of_fds_to_inherit;
	off_t handle_start = sizeof(DWORD) + number_of_fds_to_inherit;

	for (int i = 0; i < number_of_fds_to_inherit; ++i)
	{
		if (i > 2)
		{
			// We only make use of FOPEN_FLAG, FAPPEND_FLAG, FPIPE_FLAG, FDEV_FLAG.
			unsigned char flag = FOPEN_FLAG;

			if (inherit_info->fdinfo[i].flags & O_APPEND)
			{
				flag |= FAPPEND_FLAG;
			}

			if (inherit_info->fdinfo[i].type == PIPE_HANDLE)
			{
				flag |= FPIPE_FLAG;
			}
			else if (inherit_info->fdinfo[i].type == CONSOLE_HANDLE || inherit_info->fdinfo[i].type == NULL_HANDLE)
			{
				flag |= FDEV_FLAG;
			}

			((UCHAR *)(startup_info->lpReserved2 + sizeof(DWORD)))[i] = flag;
			((HANDLE *)(startup_info->lpReserved2 + handle_start))[i] = inherit_info->fdinfo[i].handle;
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
				((HANDLE *)(startup_info->lpReserved2 + handle_start))[0] = INVALID_HANDLE_VALUE;
				((UCHAR *)(startup_info->lpReserved2 + sizeof(DWORD)))[0] = 0;
				break;
			case 1:
				startup_info->hStdOutput = inherit_info->fdinfo[1].handle;
				((HANDLE *)(startup_info->lpReserved2 + handle_start))[1] = INVALID_HANDLE_VALUE;
				((UCHAR *)(startup_info->lpReserved2 + sizeof(DWORD)))[1] = 0;
				break;
			case 2:
				startup_info->hStdError = inherit_info->fdinfo[2].handle;
				((HANDLE *)(startup_info->lpReserved2 + handle_start))[2] = INVALID_HANDLE_VALUE;
				((UCHAR *)(startup_info->lpReserved2 + sizeof(DWORD)))[2] = 0;
				break;
			}
		}
	}
}

// Return the dospath only if the file has executable permissions, else return NULL.
static UNICODE_STRING *get_absolute_dospath_of_executable(const char *path)
{
	HANDLE handle;
	UNICODE_STRING *dospath = NULL;
	UNICODE_STRING *ntpath = NULL;

	ntpath = xget_absolute_ntpath(AT_FDCWD, path);
	if (ntpath == NULL)
	{
		goto finish;
	}

	// Make sure the file has execute permissions. If it doesn't an exectuable section
	// can't be created and `CreateProcessW` will fail.
	handle = just_open2(ntpath, FILE_EXECUTE, FILE_NON_DIRECTORY_FILE);
	if (handle == INVALID_HANDLE_VALUE)
	{
		// errno will be set by `just_open`.
		// EACCESS if we do not have execute access.
		// ENOENT if the file is not PRESENT.
		goto finish;
	}

	NtClose(handle);
	dospath = ntpath_to_dospath(ntpath);

finish:
	free(ntpath);
	return dospath;
}

static UNICODE_STRING *search_for_program(const char *path)
{
	size_t length;
	char *program = NULL;
	UNICODE_STRING *dospath = NULL;
	bool executable_extension_given = false;

	// First scan the path to see if there are any extensions.
	length = strlen(path);
	// Check for ".exe", ".cmd", ".bat". In this order.
	if (length - 4 > 0)
	{
		if (path[length - 4] == '.')
		{
			if (strncmp(&path[length - 3], "exe", 3) == 0 || strncmp(&path[length - 3], "cmd", 3) || strncmp(&path[length - 3], "bat", 3))
			{
				executable_extension_given = true;
			}
		}
	}

	if (executable_extension_given)
	{
		return get_absolute_dospath_of_executable(path);
	}

	// No extension was given. Append an supposed extension and search.
	program = (char *)malloc(length + 5);
	memcpy(program, path, length);

	// Given a program without an executable extension append the possible
	// ".exe", ".cmd", ".bat" in this order.
	// First try to execute what the user wants then try appending ".exe".
	// The rationale behind this is that most programs in Windows are .exes.
	// Preferring exes over any other extension makes more sense.

	// No extension
	// This maybe for shebang execution.
	// NOTE: In Windows it is possible to have execute a non exe file.
	program[length] = '\0';
	dospath = get_absolute_dospath_of_executable(path);
	if (dospath != NULL)
	{
		goto finish;
	}

	// '.exe'
	memcpy(program + length, ".exe", 5);
	dospath = get_absolute_dospath_of_executable(program);
	if (dospath != NULL)
	{
		goto finish;
	}

	// '.cmd'
	memcpy(program + length, ".cmd", 5);
	dospath = get_absolute_dospath_of_executable(program);
	if (dospath != NULL)
	{
		goto finish;
	}

	// '.bat'
	memcpy(program + length, ".bat", 5);
	dospath = get_absolute_dospath_of_executable(program);
	if (dospath != NULL)
	{
		goto finish;
	}

	// No executable file was found.

finish:
	free(program);
	return dospath;
}

UNICODE_STRING *search_path_for_program(const char *path)
{
	const char path_separator = ';';
	size_t length;
	size_t buffer_size = 256;
	char *program = NULL;
	char *PATH = NULL;
	UNICODE_STRING *dospath = NULL;

	if ( // Normal Windows way -> C:
		(isalpha(path[0]) && path[1] == ':') ||
		// Cygwin way /c
		path[0] == '/')
	{
		// An absolute path was given, no point in searching the PATH variable.
		return search_for_program(path);
	}

	PATH = getenv("PATH");

	if (PATH == NULL)
	{
		// No PATH in environment fail.
		errno = ENOENT;
		return NULL;
	}

	program = malloc(buffer_size);
	length = strlen(path);

	size_t i = 0, j = 0;
	while (PATH[i] != '\0')
	{
		if (PATH[i] == path_separator)
		{
			if (i - j + length >= buffer_size) // (+) '\', (-) ';'
			{
				// Double the buffer.
				buffer_size *= 2;
				// If the buffer is not big enough keep doubling it.
				while (i - j + length >= buffer_size)
				{
					buffer_size *= 2;
				}

				program = realloc(program, buffer_size);
			}

			// Append program to path. Eg PATH\PROGRAM
			memcpy(program, PATH + j, i - j); // Exclude ';'
			if (*(PATH + i - 1) != '\\')
			{
				// Append a slash if path component does not have one.
				program[i - j] = '\\';
				--j;
			}
			memcpy(program + (i - j), path, length + 1);
			j = i + 1;

			// Check if the program exists in this particular directory.
			dospath = search_for_program(program);
			if (dospath != NULL)
			{
				goto finish;
			}
		}

		++i;
	}

finish:
	free(program);
	return dospath;
}

static UNICODE_STRING *get_absolute_dospath_of_program(const char *path, int use_path)
{
	if (use_path)
	{
		return search_path_for_program(path);
	}

	return search_for_program(path);
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
		u16_args[i].Length = (USHORT)(argv_size - argv_used);
		u16_args[i].MaximumLength = u16_args[i].Length;

		RtlUTF8StringToUnicodeString(&u16_args[i], &u8_args[i], FALSE);

		// In Windows program arguments are separated by a ' '.
		// Change the terminating NULL to a L' '.
		wargv[(argv_used + u16_args[i].Length) / sizeof(WCHAR)] = L' ';
		argv_used += u16_args[i].Length + sizeof(WCHAR);
	}

	// Finally put in the terminating NULL
	wargv[(argv_used - sizeof(WCHAR)) / sizeof(WCHAR)] = L'\0';

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
		u16_envs[i].Length = (USHORT)(env_size - env_used);
		u16_envs[i].MaximumLength = u16_envs[i].Length;

		RtlUTF8StringToUnicodeString(&u16_envs[i], &u8_envs[i], FALSE);

		env_used += u16_envs[i].Length + sizeof(WCHAR);
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

	VALIDATE_PATH(path, ENOENT, -1);

	// TODO
	// Ignore spawn attributes for now.
	int result = -1;

	STARTUPINFOW SINFO;
	PROCESS_INFORMATION PINFO;
	UNICODE_STRING *u16_path = NULL, *u16_cwd = NULL;
	WCHAR *wpath = NULL;
	WCHAR *wargv = NULL;
	WCHAR *wenv = NULL;
	WCHAR *wcwd = NULL;

	memset(&SINFO, 0, sizeof(SINFO));
	SINFO.cb = sizeof(SINFO);
	memset(&PINFO, 0, sizeof(PINFO));

	// Convert path to UTF-16
	u16_path = get_absolute_dospath_of_program(path, use_path);
	if (u16_path == NULL)
	{
		// Appropriate errno will be set by `get_absolute_dospath_of_executable`.
		return -1;
	}
	wpath = u16_path->Buffer;

	// Convert args to UTF-16
	wargv = convert_argv_to_wargv((char *const *)argv);

	// Convert env to UTF-16
	if (env)
	{
		wenv = convert_env_to_wenv((char *const *)env);
	}

	// Perform the actions.
	int max_fd_requested = 0;
	int number_of_actions = 0;
	int num_of_open_actions = 0;
	int num_of_close_actions = 0;
	int num_of_dup2_actions = 0;
	int num_of_chdir_actions = 0;
	int num_of_fchdir_actions = 0;

	if (actions)
	{
		number_of_actions = actions->used;
	}

	for (int i = 0; i < number_of_actions; ++i)
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

	max_fd_requested = __max(max_fd_requested, (int)(_wlibc_fd_table_size - 1));
	inherit_information inherit_info;

	initialize_inherit_information(&inherit_info, max_fd_requested);

	NTSTATUS ntstatus;

	for (int i = 0; i < number_of_actions; ++i)
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
			u16_cwd = xget_absolute_dospath(AT_FDCWD, actions->actions[i].chdir_action.path);
			if (u16_cwd == NULL)
			{
				errno = ENOENT;
				goto finish;
			}
			wenv = u16_cwd->Buffer;
		}
		break;

		case fchdir_action:
		{
			u16_cwd = xget_fd_dospath(actions->actions[i].fchdir_action.fd);
			if (u16_cwd == NULL)
			{
				errno = EBADF;
				goto finish;
			}
			wenv = u16_cwd->Buffer;
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
	free(u16_path);
	free(wargv);
	free(wenv);
	free(u16_cwd);
	free(SINFO.lpReserved2);
	cleanup_inherit_information(&inherit_info, actions);

	return result;
}
