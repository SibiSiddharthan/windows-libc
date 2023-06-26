/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <internal/fcntl.h>
#include <internal/path.h>
#include <internal/spawn.h>
#include <internal/validate.h>
#include <errno.h>
#include <fcntl.h>
#include <spawn.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

int do_open(int dirfd, const char *name, int oflags, mode_t perm);
int adjust_priority_of_process(HANDLE process, KPRIORITY new_priority);

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

static int initialize_inherit_information(inherit_information *info, int max_fd)
{
	info->fdinfo = (inherit_fdinfo *)RtlAllocateHeap(NtCurrentProcessHeap(), 0, sizeof(inherit_fdinfo) * (max_fd + 1));
	if (info->fdinfo == NULL)
	{
		errno = ENOMEM;
		return -1;
	}

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
			info->fdinfo[i].handle = NULL;
			info->fdinfo[i].type = 0;
			info->fdinfo[i].flags = 0;
		}
	}
	SHARED_UNLOCK_FD_TABLE();

	return 0;
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

	RtlFreeHeap(NtCurrentProcessHeap(), 0, info->fdinfo);
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
static int give_inherit_information_to_startupinfo(inherit_information *inherit_info, STARTUPINFOW *startup_info, VOID **buffer)
{
	// First count the maximum fd that is to be inherited.
	int number_of_fds_to_inherit = 0;
	int real_max_fd = 0;

	for (int i = 0; i <= inherit_info->fds; ++i)
	{
		if (inherit_info->fdinfo[i].handle != NULL)
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
	*buffer = RtlAllocateHeap(NtCurrentProcessHeap(), 0, startup_info->cbReserved2);
	startup_info->lpReserved2 = *buffer;

	if (*buffer == NULL)
	{
		errno = ENOMEM;
		return -1;
	}

	*(DWORD *)startup_info->lpReserved2 = number_of_fds_to_inherit;
	off_t handle_start = sizeof(DWORD) + number_of_fds_to_inherit;

	for (int i = 0; i < number_of_fds_to_inherit; ++i)
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

		// Pass the invalid handles as INVALID_HANDLE_VALUE to maintain compatibility with msvcrt.
		((UCHAR *)(startup_info->lpReserved2 + sizeof(DWORD)))[i] = flag;
		((HANDLE *)(startup_info->lpReserved2 + handle_start))[i] =
			inherit_info->fdinfo[i].handle != NULL ? inherit_info->fdinfo[i].handle : INVALID_HANDLE_VALUE;

		// Give the std handles to startupinfo also.
		switch (i)
		{
		case 0:
			startup_info->hStdInput = inherit_info->fdinfo[i].handle;
			break;
		case 1:
			startup_info->hStdOutput = inherit_info->fdinfo[i].handle;
			break;
		case 2:
			startup_info->hStdError = inherit_info->fdinfo[i].handle;
			break;
		default:
			break;
		}
	}

	return 0;
}

// Return the dospath only if the file has executable permissions, else return NULL.
static UNICODE_STRING *get_absolute_dospath_of_executable(const char *path)
{
	HANDLE handle;
	UNICODE_STRING *dospath = NULL;
	UNICODE_STRING *ntpath = NULL;

	ntpath = get_absolute_ntpath(AT_FDCWD, path);
	if (ntpath == NULL)
	{
		goto finish;
	}

	// Make sure the file has execute permissions. If it doesn't an exectuable section
	// can't be created and `CreateProcessW` will fail.
	handle = just_open2(ntpath, FILE_EXECUTE, FILE_NON_DIRECTORY_FILE);
	if (handle == NULL)
	{
		// errno will be set by `just_open`.
		// EACCESS if we do not have execute access.
		// ENOENT if the file is not PRESENT.
		goto finish;
	}

	NtClose(handle);
	dospath = ntpath_to_dospath(ntpath);

finish:
	RtlFreeHeap(NtCurrentProcessHeap(), 0, ntpath);
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
	if (length > 4)
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
	program = (char *)RtlAllocateHeap(NtCurrentProcessHeap(), 0, length + 5);
	if (program == NULL)
	{
		errno = ENOMEM;
		return NULL;
	}

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
	RtlFreeHeap(NtCurrentProcessHeap(), 0, program);
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

	program = RtlAllocateHeap(NtCurrentProcessHeap(), 0, buffer_size);
	if (program == NULL)
	{
		errno = ENOMEM;
		return NULL;
	}

	length = strlen(path);

	size_t i = 0, j = 0;
	while (1)
	{
		if (PATH[i] == path_separator || PATH[i] == '\0')
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

				program = RtlReAllocateHeap(NtCurrentProcessHeap(), 0, program, buffer_size);
				if (program == NULL)
				{
					errno = ENOMEM;
					return NULL;
				}
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

		if (PATH[i] == '\0')
		{
			break;
		}

		++i;
	}

finish:
	RtlFreeHeap(NtCurrentProcessHeap(), 0, program);
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

static char *convert_argv_to_windows_cmd(const char *arg, size_t *size)
{
	bool has_whitespaces = false;
	bool arg_needs_to_be_modified = false;
	size_t number_of_double_quotes = 0;
	size_t number_of_backslashes_before_quote = 0;
	size_t number_of_backslashes_at_the_end = 0;

	*size = 0;

	for (size_t i = 0; arg[i] != '\0'; ++i)
	{
		if (arg[i] == ' ' || arg[i] == '\t')
		{
			has_whitespaces = true;
		}

		if (arg[i] == '"')
		{
			++number_of_double_quotes;
		}

		if (arg[i] == '\\')
		{
			size_t count = i;
			++i;

			while (arg[i] == '\\')
			{
				++i;
			}

			count = i - count;
			*size += count;

			if (arg[i] == '"')
			{
				number_of_backslashes_before_quote += count;
				++number_of_double_quotes;
				++(*size);
				continue;
			}

			if (arg[i] == '\0')
			{
				number_of_backslashes_at_the_end = count;
				break;
			}

			--i;
			continue;
		}

		++(*size);
	}

	if (has_whitespaces || number_of_double_quotes > 0)
	{
		arg_needs_to_be_modified = true;
		*size += 2;
	}

	if (!arg_needs_to_be_modified)
	{
		// Given arg can be placed in the command line string without any modification.
		return NULL;
	}

	// Each double quote in arg needs to be preceded by a backslash.
	*size += number_of_double_quotes;

	// Each consecutive backslash before a double quote needs to be escaped as well. (N backslashes + " -> 2N + 1 backslashes + ")
	*size += number_of_backslashes_before_quote;

	// If we are quoting the arg and there happens to be a backslash(es) at the end they need to be escaped.
	*size += number_of_backslashes_at_the_end;

	size_t new_arg_index = 0;
	char *new_arg = NULL;

	new_arg = (char *)RtlAllocateHeap(NtCurrentProcessHeap(), 0, *size + 1);
	if (new_arg == NULL)
	{
		errno = ENOMEM;
		return (char *)-1; // Dirty hack to catch errors here.
	}

	new_arg[new_arg_index++] = '"';
	for (size_t i = 0; arg[i] != '\0'; ++i)
	{
		if (arg[i] == '"')
		{
			new_arg[new_arg_index++] = '\\';
			new_arg[new_arg_index++] = '"';
			continue;
		}

		if (arg[i] == '\\')
		{
			size_t count = i;

			new_arg[new_arg_index++] = '\\';
			++i;

			while (arg[i] == '\\')
			{
				++i;
				new_arg[new_arg_index++] = '\\';
			}

			count = i - count;

			if (arg[i] == '"')
			{
				for (size_t j = 0; j < count; ++j)
				{
					new_arg[new_arg_index++] = '\\';
				}

				new_arg[new_arg_index++] = '\\';
				new_arg[new_arg_index++] = '"';

				continue;
			}

			--i;
			continue;
		}

		new_arg[new_arg_index++] = arg[i];
	}

	for (size_t i = 0; i < number_of_backslashes_at_the_end; ++i)
	{
		new_arg[new_arg_index++] = '\\';
	}

	new_arg[new_arg_index++] = '"';
	new_arg[new_arg_index++] = '\0';

	return new_arg;
}

static UNICODE_STRING *shebang_get_executable_and_args(const UNICODE_STRING *dospath)
{
	NTSTATUS status;
	IO_STATUS_BLOCK io;
	HANDLE handle;
	UNICODE_STRING *ntpath;

	UNICODE_STRING *dos_shebang_exe = NULL;
	UNICODE_STRING *dos_shebang_exe_with_args = NULL;
	char *shebang_arg_normalized = NULL;
	char *shebang_exe = NULL;

	char *line_buffer = NULL;
	size_t line_buffer_size = 4096;
	size_t length_of_first_line = 0;
	size_t position_of_first_character_after_shebang = 0;

	// Execution flow enters here only in the case of a shebang execution.
	// The dospath will be valid and it will exist.
	ntpath = dospath_to_ntpath(dospath);
	if (ntpath == NULL)
	{
		// errno will be set by `dospath_to_ntpath`.
		return NULL;
	}

	handle = just_open2(ntpath, FILE_READ_DATA | SYNCHRONIZE, FILE_SYNCHRONOUS_IO_NONALERT);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, ntpath);

	if (handle == NULL)
	{
		// This should not happen as the file exists. Just in case.
		return NULL;
	}

	line_buffer = RtlAllocateHeap(NtCurrentProcessHeap(), HEAP_ZERO_MEMORY, line_buffer_size);
	if (line_buffer == NULL)
	{
		errno = ENOMEM;
		return NULL;
	}

	// Read the first line of the file.
	status = NtReadFile(handle, NULL, NULL, NULL, &io, line_buffer, 4096, NULL, NULL);
	if (status != STATUS_SUCCESS && status != STATUS_END_OF_FILE)
	{
		// No read access.
		errno = EACCES;
		return NULL;
	}

	if (!(line_buffer[0] == '#' && line_buffer[1] == '!'))
	{
		// Not shebang.
		errno = ENOEXEC;
		return NULL;
	}

	// Find where the first line ends.
	for (size_t i = 2; i < io.Information; ++i)
	{
		if (line_buffer[i] == '\n' || line_buffer[i] == '\r')
		{
			length_of_first_line = i;
			break;
		}
	}

	if (length_of_first_line == 0)
	{
		if (io.Information == 4096)
		{
			// Need to read more data.
			// Read 32768 bytes. This is the command line limit on Windows.
			char *temp = RtlReAllocateHeap(NtCurrentProcessHeap(), HEAP_ZERO_MEMORY, line_buffer, 32768);
			if (temp == NULL)
			{
				errno = ENOMEM;
				goto finish;
			}

			line_buffer = temp;
			line_buffer_size = 32768;

			status = NtReadFile(handle, NULL, NULL, NULL, &io, line_buffer + 4096, 32768 - 4096, NULL, NULL);
			if (status != STATUS_SUCCESS && status != STATUS_END_OF_FILE)
			{
				goto finish;
			}

			// Find where the first line ends.
			for (size_t i = 4096; i < io.Information + 4096; ++i)
			{
				if (line_buffer[i] == '\n' || line_buffer[i] == '\r')
				{
					length_of_first_line = i;
					break;
				}
			}

			if (io.Information == 0)
			{
				length_of_first_line = 4096;
			}

			if (length_of_first_line == 0)
			{
				errno = E2BIG;
				goto finish;
			}
		}
		else
		{
			// File has only one line and that line contains "#! ...".
			length_of_first_line = io.Information;
		}
	}

	// NULL terminate the line buffer. This will not cause heap corruption.
	if (length_of_first_line < line_buffer_size)
	{
		line_buffer[length_of_first_line] = '\0';
	}
	else
	{
		// Ignore the last character.
		line_buffer[length_of_first_line - 1] = '\0';
	}

	// We don't need the file to open anymore. Close it.
	NtClose(handle);

	// Get the command line, after "#!".
	for (size_t i = 2; i < length_of_first_line; ++i)
	{
		if (line_buffer[i] == ' ' || line_buffer[i] == '\t')
		{
			// There might be several white spaces after "#!", skip them.
			continue;
		}

		else
		{
			position_of_first_character_after_shebang = i;
			break;
		}
	}

	if (position_of_first_character_after_shebang == 0)
	{
		// File only has "#!" plus zero or more whitespaces in its first line.
		errno = ENOEXEC;
		goto finish;
	}

	// In Unix systems the executables are usually given as /bin/sh, /usr/bin/sh, etc.
	// In Windows there is no proper root directory, so only consider the last component (sh)
	// and try to find it in the PATH.
	size_t start_of_last_component_of_exe = position_of_first_character_after_shebang;
	size_t end_of_last_component_of_exe = length_of_first_line - 1;
	// bool shebang_exe_is_quoted = line_buffer[position_of_first_character_after_shebang] == '"';

	for (size_t i = position_of_first_character_after_shebang; i < length_of_first_line; ++i)
	{
		if (line_buffer[i] == ' ' || line_buffer[i] == '\t')
		{
			end_of_last_component_of_exe = i - 1;
			break;
		}

		if (line_buffer[i] == '/')
		{
			start_of_last_component_of_exe = i + 1;
		}
	}

	// Copy the last component to the buffer.
	shebang_exe = (char *)RtlAllocateHeap(NtCurrentProcessHeap(), 0, end_of_last_component_of_exe - start_of_last_component_of_exe + 2);
	if (shebang_exe == NULL)
	{
		errno = ENOMEM;
		goto finish;
	}

	memcpy(shebang_exe, line_buffer + start_of_last_component_of_exe, end_of_last_component_of_exe - start_of_last_component_of_exe + 1);
	shebang_exe[end_of_last_component_of_exe - start_of_last_component_of_exe + 1] = '\0';

	dos_shebang_exe = search_path_for_program(shebang_exe);
	if (dos_shebang_exe == NULL)
	{
		if (errno != ENOENT)
		{
			goto finish;
		}

		// If we can't find the specified program in the path, search for it in the
		// cuurent working directory.
		errno = 0;
		dos_shebang_exe = search_for_program(shebang_exe);

		if (dos_shebang_exe == NULL)
		{
			goto finish;
		}
	}

	size_t start_of_args = 0;

	for (size_t i = end_of_last_component_of_exe + 2; i < length_of_first_line; ++i)
	{
		if (line_buffer[i] != ' ' && line_buffer[i] != '\t')
		{
			start_of_args = i;
			break;
		}
	}

	size_t shebang_arg_normalized_size = 0;
	size_t dos_shebang_exe_with_args_used = 0;
	UTF8_STRING u8_shebang_arg = {0, 0, NULL};
	UNICODE_STRING u16_shebang_arg = {0, 0, NULL};

	if (start_of_args != 0)
	{
		shebang_arg_normalized = convert_argv_to_windows_cmd(line_buffer + start_of_args, &shebang_arg_normalized_size);
		if (shebang_arg_normalized == (char *)-1)
		{
			goto finish;
		}

		u8_shebang_arg.Buffer = shebang_arg_normalized == NULL ? &line_buffer[start_of_args] : shebang_arg_normalized;
		u8_shebang_arg.Length = (USHORT)shebang_arg_normalized_size;
		u8_shebang_arg.MaximumLength = u8_shebang_arg.Length + 1;

		status = RtlUTF8StringToUnicodeString(&u16_shebang_arg, &u8_shebang_arg, TRUE);
		if (status != STATUS_SUCCESS)
		{
			map_ntstatus_to_errno(status);
			goto finish;
		}
	}

	dos_shebang_exe_with_args =
		(UNICODE_STRING *)RtlAllocateHeap(NtCurrentProcessHeap(), 0,
										  sizeof(UNICODE_STRING) + dos_shebang_exe->MaximumLength + u16_shebang_arg.MaximumLength +
											  dospath->MaximumLength + sizeof(WCHAR)); // For an extra NULL if no args are given.

	if (dos_shebang_exe_with_args == NULL)
	{
		errno = ENOMEM;
		goto finish;
	}

	// executable
	memcpy((CHAR *)dos_shebang_exe_with_args + sizeof(UNICODE_STRING), dos_shebang_exe->Buffer, dos_shebang_exe->MaximumLength);
	dos_shebang_exe_with_args_used += dos_shebang_exe->MaximumLength;

	// args
	if (u16_shebang_arg.MaximumLength > 0)
	{
		memcpy((CHAR *)dos_shebang_exe_with_args + sizeof(UNICODE_STRING) + dos_shebang_exe_with_args_used, u16_shebang_arg.Buffer,
			   u16_shebang_arg.MaximumLength);
		dos_shebang_exe_with_args_used += u16_shebang_arg.MaximumLength;

		RtlFreeUnicodeString(&u16_shebang_arg);
	}
	else
	{
		// Put a NULL if no args are given inside the shebang script.
		*(WCHAR *)((CHAR *)dos_shebang_exe_with_args + sizeof(UNICODE_STRING) + dos_shebang_exe_with_args_used) = L'\0';
		dos_shebang_exe_with_args_used += sizeof(WCHAR);
	}

	// filename
	memcpy((CHAR *)dos_shebang_exe_with_args + sizeof(UNICODE_STRING) + dos_shebang_exe_with_args_used, dospath->Buffer,
		   dospath->MaximumLength);
	dos_shebang_exe_with_args_used += dospath->MaximumLength;

	dos_shebang_exe_with_args->Buffer = (WCHAR *)((CHAR *)dos_shebang_exe_with_args + sizeof(UNICODE_STRING));
	dos_shebang_exe_with_args->Length = (USHORT)dos_shebang_exe_with_args_used;
	dos_shebang_exe_with_args->MaximumLength = (USHORT)dos_shebang_exe_with_args_used;

finish:
	RtlFreeHeap(NtCurrentProcessHeap(), 0, shebang_arg_normalized);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, shebang_exe);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, dos_shebang_exe);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, line_buffer);

	return dos_shebang_exe_with_args;
}

static WCHAR *convert_argv_to_wargv(char *const argv[], size_t *size)
{
	NTSTATUS status;
	UNICODE_STRING *u16_args = NULL;
	UTF8_STRING *u8_args = NULL;
	WCHAR *wargv = NULL;

	int argc = 0;
	size_t argv_size = 0;
	size_t argv_used = 0;
	char *const *pargv = argv;
	char **argv_normalized = NULL;

	*size = -1ull;

	// Count the number of elements in argv.
	while (*argv)
	{
		++argc;
		++argv;
	}
	argv = pargv;

	if (argc == 0)
	{
		// Highly unlikely, but not an error.
		*size = 0;
		return NULL;
	}

	u8_args = (UTF8_STRING *)RtlAllocateHeap(NtCurrentProcessHeap(), 0, sizeof(UTF8_STRING) * argc);
	u16_args = (UNICODE_STRING *)RtlAllocateHeap(NtCurrentProcessHeap(), 0, sizeof(UNICODE_STRING) * argc);
	argv_normalized = (char **)RtlAllocateHeap(NtCurrentProcessHeap(), HEAP_ZERO_MEMORY, sizeof(char *) * argc);

	if (argc != 0 && (u8_args == NULL || u16_args == NULL || argv_normalized == NULL))
	{
		errno = ENOMEM;
		return NULL;
	}

	for (int i = 0; i < argc; ++i)
	{
		char *new_arg;
		size_t arg_size;

		new_arg = convert_argv_to_windows_cmd(argv[i], &arg_size);
		if (new_arg == (char *)-1)
		{
			goto finish;
		}

		argv_normalized[i] = new_arg;

		u8_args[i].Buffer = new_arg == NULL ? argv[i] : argv_normalized[i];
		u8_args[i].Length = (USHORT)arg_size;
		u8_args[i].MaximumLength = u8_args[i].Length + 1;

		argv_size += u8_args[i].MaximumLength;
	}

	argv_size *= sizeof(WCHAR);
	wargv = (WCHAR *)RtlAllocateHeap(NtCurrentProcessHeap(), 0, argv_size);

	if (wargv == NULL)
	{
		errno = ENOMEM;
		goto finish;
	}

	for (int i = 0; i < argc; ++i)
	{
		u16_args[i].Buffer = (WCHAR *)((char *)wargv + argv_used);
		u16_args[i].Length = (USHORT)(argv_size - argv_used);
		u16_args[i].MaximumLength = u16_args[i].Length;

		status = RtlUTF8StringToUnicodeString(&u16_args[i], &u8_args[i], FALSE);
		if (status != STATUS_SUCCESS)
		{
			map_ntstatus_to_errno(status);
			goto finish;
		}

		// In Windows program arguments are separated by a ' '.
		// Change the terminating NULL to a L' '.
		wargv[(argv_used + u16_args[i].Length) / sizeof(WCHAR)] = L' ';
		argv_used += u16_args[i].Length + sizeof(WCHAR);
	}

	// Finally put in the terminating NULL
	wargv[(argv_used - sizeof(WCHAR)) / sizeof(WCHAR)] = L'\0';
	argv_used += sizeof(WCHAR);

	*size = argv_size;

finish:
	RtlFreeHeap(NtCurrentProcessHeap(), 0, u16_args);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, u8_args);

	for (int i = 0; i < argc; ++i)
	{
		// Some pointers may be NULL here.
		// NOTE: RtlFreeHeap(..., NULL) -> nop.
		RtlFreeHeap(NtCurrentProcessHeap(), 0, argv_normalized[i]);
	}

	return wargv;
}

static WCHAR *prepend_shebang_to_wargv(const WCHAR *old_argv, size_t old_size, const WCHAR *shebang_argv, size_t shebang_size,
									   size_t *new_size)
{
	// shebang argv consists of 3 args, the executable, args(normalized), filename. Each of them is separated by a NULL.
	WCHAR *new_argv = NULL;
	WCHAR *final_argv = NULL;

	bool quote_executable = false;
	bool quote_filename = false;

	size_t index = 0;
	size_t executable_start = 0;
	size_t executable_end = 0;
	size_t arg_start = 0;
	size_t arg_end = 0;
	size_t filename_start = 0;
	size_t filename_end = 0;
	size_t new_argv_used = 0;

	// executable
	while (shebang_argv[index] != L'\0')
	{
		if (shebang_argv[index] == L' ')
		{
			quote_executable = true;
		}
		++index;
	}
	executable_end = index * sizeof(WCHAR);
	++index;

	// args
	arg_start = index * sizeof(WCHAR);
	while (shebang_argv[index] != L'\0')
	{
		++index;
	}
	arg_end = index * sizeof(WCHAR);
	++index;

	// filename
	filename_start = index * sizeof(WCHAR);
	while (shebang_argv[index] != L'\0')
	{
		if (shebang_argv[index] == L' ')
		{
			quote_executable = true;
		}
		++index;
	}
	filename_end = index * sizeof(WCHAR);

	if (quote_executable)
	{
		shebang_size += 2 * sizeof(WCHAR);
	}

	if (quote_filename)
	{
		shebang_size += 2 * sizeof(WCHAR);
	}

	new_argv = (WCHAR *)RtlAllocateHeap(NtCurrentProcessHeap(), 0, shebang_size);
	if (new_argv == NULL)
	{
		errno = ENOMEM;
		return NULL;
	}

	// executable
	if (quote_executable)
	{
		*new_argv = L'"';
		new_argv_used += sizeof(WCHAR);
	}

	memcpy((CHAR *)new_argv + new_argv_used, (CHAR *)shebang_argv + executable_start, executable_end - executable_start);
	new_argv_used += executable_end - executable_start;

	if (quote_executable)
	{
		*(WCHAR *)((CHAR *)new_argv + new_argv_used) = L'"';
		new_argv_used += sizeof(WCHAR);
	}

	*(WCHAR *)((CHAR *)new_argv + new_argv_used) = L' ';
	new_argv_used += sizeof(WCHAR);

	// argv
	if (arg_end - arg_start > 0)
	{
		memcpy((CHAR *)new_argv + new_argv_used, (CHAR *)shebang_argv + arg_start, arg_end - arg_start);
		new_argv_used += arg_end - arg_start;

		*(WCHAR *)((CHAR *)new_argv + new_argv_used) = L' ';
		new_argv_used += sizeof(WCHAR);
	}

	// filename
	if (quote_filename)
	{
		*new_argv = L'"';
		new_argv_used += sizeof(WCHAR);
	}

	memcpy((CHAR *)new_argv + new_argv_used, (CHAR *)shebang_argv + filename_start, filename_end - filename_start);
	new_argv_used += filename_end - filename_start;

	if (quote_filename)
	{
		*(WCHAR *)((CHAR *)new_argv + new_argv_used) = L'"';
		new_argv_used += sizeof(WCHAR);
	}

	// Append a space always so we can concat straight away.
	*(WCHAR *)((CHAR *)new_argv + new_argv_used) = L' ';
	new_argv_used += sizeof(WCHAR);

	// When prepending we need to skip arg0.
	size_t start_of_arg1 = 0;
	size_t old_argv_needed = old_size;
	short quote_count = 0;
	bool quoted_arg0 = false;

	if (old_argv != NULL)
	{
		for (size_t i = 0; old_argv[i] != L'\0'; ++i)
		{
			if (i == 0)
			{
				if (old_argv[i] == L'"')
				{
					quoted_arg0 = true;
					++quote_count;
				}

				continue;
			}

			if (quoted_arg0)
			{
				// This is a bit involved.
				// We have already read the first '"'. If we read another bare '"'. The start will be i+2 (after a space).
				// If we read a '\' followed by a '"', do not count that.

				if (old_argv[i] == L'"')
				{
					// When only one arg is given, this assignment will exceed old_size. We are constraining after the loop.
					start_of_arg1 = i + 2;
					break;
				}

				if (old_argv[i] == L'\\' && old_argv[i + 1] == L'"')
				{
					++i;
					continue;
				}
			}
			else
			{
				// Easy. Just find the first ' '.
				if (old_argv[i] == L' ')
				{
					start_of_arg1 = i + 1;
					break;
				}
			}
		}
	}

	start_of_arg1 *= sizeof(WCHAR);

	// Sanity constraint.
	if (start_of_arg1 > old_size)
	{
		start_of_arg1 = 0;
	}

	if (old_size > 0) // old_argv is not NULL.
	{
		old_argv_needed = start_of_arg1 == 0 ? 0 : old_size - start_of_arg1;
	}

	final_argv =
		(WCHAR *)RtlAllocateHeap(NtCurrentProcessHeap(), 0, new_argv_used + old_argv_needed + sizeof(WCHAR)); // Terminating NULL if needed.
	if (final_argv == NULL)
	{
		errno = ENOMEM;
		RtlFreeHeap(NtCurrentProcessHeap(), 0, new_argv);
		return NULL;
	}

	memcpy(final_argv, new_argv, new_argv_used);

	if (old_argv_needed > 0)
	{
		// This will copy the terminating NULL of old_argv also.
		memcpy((CHAR *)final_argv + new_argv_used, (CHAR *)old_argv + start_of_arg1, old_argv_needed);
	}
	else
	{
		// Put the NULL if we don't use old_argv at all.
		*(WCHAR *)((CHAR *)final_argv + new_argv_used) = L'\0';
	}

	RtlFreeHeap(NtCurrentProcessHeap(), 0, new_argv);

	*new_size = new_argv_used + old_argv_needed;

	return final_argv;
}

static WCHAR *convert_env_to_wenv(char *const env[], size_t *size)
{
	NTSTATUS status;
	UNICODE_STRING *u16_envs = NULL;
	UTF8_STRING *u8_envs = NULL;
	WCHAR *wenv = NULL;

	int envc = 0;
	size_t env_size = 0;
	size_t env_used = 0;
	char *const *penv = env;

	*size = -1ull;

	// Count the number of elements in env.
	while (*env)
	{
		++envc;
		++env;
	}
	env = penv;

	u8_envs = (UTF8_STRING *)RtlAllocateHeap(NtCurrentProcessHeap(), 0, sizeof(UTF8_STRING) * envc);
	u16_envs = (UNICODE_STRING *)RtlAllocateHeap(NtCurrentProcessHeap(), 0, sizeof(UNICODE_STRING) * envc);

	if (u8_envs == NULL || u16_envs == NULL)
	{
		errno = ENOMEM;
		return NULL;
	}

	for (int i = 0; i < envc; ++i)
	{
		RtlInitUTF8String(&u8_envs[i], env[i]);
		env_size += u8_envs[i].MaximumLength;
	}

	++env_size; // For the terminating 2 NULLs.
	env_size *= sizeof(WCHAR);
	wenv = (WCHAR *)RtlAllocateHeap(NtCurrentProcessHeap(), 0, env_size);

	if (wenv == NULL)
	{
		errno = ENOMEM;
		goto finish;
	}

	for (int i = 0; i < envc; ++i)
	{
		u16_envs[i].Buffer = (WCHAR *)((char *)wenv + env_used);
		u16_envs[i].Length = (USHORT)(env_size - env_used);
		u16_envs[i].MaximumLength = u16_envs[i].Length;

		status = RtlUTF8StringToUnicodeString(&u16_envs[i], &u8_envs[i], FALSE);
		if (status != STATUS_SUCCESS)
		{
			map_ntstatus_to_errno(status);
			goto finish;
		}

		env_used += u16_envs[i].Length + sizeof(WCHAR);
	}

	// Finally put in the terminating NULL.
	// NOTE: Since 'RtlUTF8StringToUnicodeString' appends a terminating NULL to the converted string,
	// we only need to put the other terminating NULL to denote the end of the environment.
	wenv[env_used / sizeof(WCHAR)] = L'\0';

	*size = env_size;

finish:
	RtlFreeHeap(NtCurrentProcessHeap(), 0, u16_envs);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, u8_envs);

	return wenv;
}

int wlibc_common_spawn(pid_t *restrict pid, const char *restrict path, const spawn_actions_t *restrict actions,
					   const spawnattr_t *restrict attributes, int use_path, char *restrict const argv[], char *restrict const env[])
{
	int result = -1;

	UNICODE_STRING *u16_path = NULL, *u16_cwd = NULL;
	WCHAR *wpath = NULL;
	WCHAR *wargv = NULL;
	WCHAR *wenv = NULL;
	WCHAR *wcwd = NULL;
	VOID *inherit_info_buffer = NULL;
	size_t wargv_size, wenv_size;

	// Initialize this first as the cleanup label 'finish' requires it.
	inherit_information inherit_info = {0, NULL};

	// Validations.
	VALIDATE_PATH(path, ENOENT, -1);
	VALIDATE_PTR(argv, EINVAL, -1);

	// Convert path to UTF-16
	u16_path = get_absolute_dospath_of_program(path, use_path);
	if (u16_path == NULL)
	{
		// Appropriate errno will be set by `get_absolute_dospath_of_executable`.
		goto finish;
	}
	wpath = u16_path->Buffer;

	// Convert args to UTF-16.
	wargv = convert_argv_to_wargv((char *const *)argv, &wargv_size);

	if (wargv_size == -1ull) // Out of memory error.
	{
		goto finish;
	}

	// The arguments have exceeded the command line limit on Windows.
	if (wargv_size >= 65536)
	{
		errno = E2BIG;
		goto finish;
	}

	// Convert env to UTF-16.
	if (env)
	{
		wenv = convert_env_to_wenv((char *const *)env, &wenv_size);

		if (wenv_size == -1ull) // Out of memory error.
		{
			goto finish;
		}
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

		if (num_of_chdir_actions + num_of_fchdir_actions > 2)
		{
			// More than one directory change requested.
			errno = EINVAL;
			goto finish;
		}
	}

	max_fd_requested = __max(max_fd_requested, (int)(_wlibc_fd_table_size - 1));

	if (initialize_inherit_information(&inherit_info, max_fd_requested) != 0)
	{
		goto finish;
	}

	NTSTATUS ntstatus;

	for (int i = 0; i < number_of_actions; ++i)
	{
		switch (actions->actions[i].type)
		{

		case open_action:
		{
			// TODO make this more efficient.
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
			// TODO make this more efficient.
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

			inherit_info.fdinfo[fd].handle = NULL;
			inherit_info.fdinfo[fd].flags = 0;
			inherit_info.fdinfo[fd].type = 0;
		}
		break;

		case dup2_action:
		{
			// TODO make this more efficient.
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
			u16_cwd = get_absolute_dospath(AT_FDCWD, actions->actions[i].chdir_action.path);
			if (u16_cwd == NULL)
			{
				// errno will be set by `get_absolute_dospath`.
				goto finish;
			}
			wcwd = u16_cwd->Buffer;
		}
		break;

		case fchdir_action:
		{
			u16_cwd = get_fd_dospath(actions->actions[i].fchdir_action.fd);
			if (u16_cwd == NULL)
			{
				errno = EBADF;
				goto finish;
			}
			wcwd = u16_cwd->Buffer;
		}
		break;
		}
	}

	STARTUPINFOW sinfo = {0};
	PROCESS_INFORMATION pinfo = {0};
	DWORD priority_class = NORMAL_PRIORITY_CLASS;

	sinfo.cb = sizeof(STARTUPINFOW);

	if (give_inherit_information_to_startupinfo(&inherit_info, &sinfo, &inherit_info_buffer) != 0)
	{
		goto finish;
	}

	if (attributes)
	{
		if (attributes->flags & POSIX_SPAWN_SETSCHEDULER)
		{
			switch (attributes->schedpolicy)
			{
			case SCHED_IDLE:
				priority_class = IDLE_PRIORITY_CLASS;
				break;
			case SCHED_RR:
				priority_class = NORMAL_PRIORITY_CLASS;
				break;
			case SCHED_FIFO:
				priority_class = HIGH_PRIORITY_CLASS;
				break;
			case SCHED_BATCH:
				priority_class = BELOW_NORMAL_PRIORITY_CLASS;
				break;
			case SCHED_SPORADIC:
				priority_class = ABOVE_NORMAL_PRIORITY_CLASS;
				break;
			default: // Should be unreachable.
				priority_class = NORMAL_PRIORITY_CLASS;
			}
		}
	}

	// Do the spawn.
	BOOL status = CreateProcessW(wpath, wargv, NULL, NULL, TRUE, CREATE_UNICODE_ENVIRONMENT | priority_class, wenv, wcwd, &sinfo, &pinfo);
	if (status == 0)
	{
		DWORD error = GetLastError();
		if (error != ERROR_BAD_EXE_FORMAT)
		{
			map_doserror_to_errno(error);
			goto finish;
		}

		// Perform a shebang spawn.
		WCHAR *shebang_argv = NULL;
		UNICODE_STRING *u16_path_and_args = shebang_get_executable_and_args(u16_path);
		if (u16_path_and_args == NULL)
		{
			goto finish;
		}

		shebang_argv =
			prepend_shebang_to_wargv(wargv, wargv_size, u16_path_and_args->Buffer, u16_path_and_args->MaximumLength, &wargv_size);

		RtlFreeHeap(NtCurrentProcessHeap(), 0, u16_path);
		RtlFreeHeap(NtCurrentProcessHeap(), 0, wargv);

		u16_path = u16_path_and_args;
		wpath = u16_path->Buffer; // The executable will be NULL separated.
		wargv = shebang_argv;

		// Exceeding the command line limit.
		if (wargv_size >= 65535)
		{
			errno = E2BIG;
			goto finish;
		}

		status = CreateProcessW(wpath, wargv, NULL, NULL, TRUE, CREATE_UNICODE_ENVIRONMENT | priority_class, wenv, wcwd, &sinfo, &pinfo);

		// Shebang spawn failed.
		if (status == 0)
		{
			map_doserror_to_errno(GetLastError());
			goto finish;
		}
	}

	if (attributes)
	{
		if (attributes->flags & POSIX_SPAWN_SETSCHEDPARAM && attributes->schedpriority != 0)
		{
			// Don't check for errors here.
			adjust_priority_of_process(pinfo.hProcess, attributes->schedpriority);
		}
	}

	// Add the child information to our process table.
	if (add_child(pinfo.dwProcessId, pinfo.hProcess) != 0)
	{
		goto finish;
	}

	*pid = pinfo.dwProcessId;
	result = 0;

finish:
	RtlFreeHeap(NtCurrentProcessHeap(), 0, u16_path);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, wargv);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, wenv);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, u16_cwd);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, inherit_info_buffer);
	cleanup_inherit_information(&inherit_info, actions);

	return result;
}
