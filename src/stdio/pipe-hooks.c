/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <stdio-hooks.h>
#include <io.h>
#include <fcntl_internal.h>
#include <fcntl.h>
#include <Windows.h>
#include <string.h>
#include <stdlib.h>

int parse_mode(const char *mode);

FILE *wlibc_popen(const char *command, const char *mode)
{
	int length = strlen(command) + 1;
	char *win_command = (char *)malloc(sizeof(char) * length);
	strcpy(win_command, command);
	char *temp = win_command;
	// Iterate the first argument only, convert any forward slash to back slash for cmd.exe
	while (*win_command != '\0' && *win_command != ' ')
	{
		if (*win_command == '/')
		{
			*win_command = '\\';
		}
		++win_command;
	}
	win_command = temp;

	FILE *_PIPE = _popen(win_command, mode);
	free(win_command);

	if (_PIPE == NULL)
	{
		return NULL;
	}
	int flags = parse_mode(mode);
	HANDLE pipe_handle = (HANDLE)_get_osfhandle(_fileno(_PIPE));
	register_to_fd_table(pipe_handle, NULL, PIPE_HANDLE, flags);
	return _PIPE;
}

int wlibc_pclose(FILE *stream)
{
	int status = -1;
	if (stream)
	{
		HANDLE pipe_handle = (HANDLE)_get_osfhandle(_fileno(stream));
		status = _pclose(stream); // Underlying OS handle is closed here
		unregister_from_fd_table(pipe_handle);
	}

	return status;
}
