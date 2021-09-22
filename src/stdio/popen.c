/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <Windows.h>
#include <internal/process.h>
#include <fcntl.h>
#include <internal/fcntl.h>
#include <internal/error.h>

int parse_mode(const char *mode);
int get_buf_mode(int flags);

FILE *wlibc_popen(const char *command, const char *mode)
{
	if (command == NULL || mode == NULL)
	{
		errno = EINVAL;
		return NULL;
	}

	int pmode = parse_mode(mode);
	if (pmode & O_RDWR)
	{
		errno = EINVAL;
		return NULL;
	}

	pmode = (pmode & O_WRONLY ? O_WRONLY : O_RDONLY);

	HANDLE read_end, write_end;
	BOOL status;
	SECURITY_ATTRIBUTES SA;
	SA.nLength = sizeof(SECURITY_ATTRIBUTES);
	SA.bInheritHandle = TRUE;
	SA.lpSecurityDescriptor = NULL;
	status = CreatePipe(&read_end, &write_end, &SA, 4096);
	if (status == 0)
	{
		map_win32_error_to_wlibc(GetLastError());
		return NULL;
	}
	if (pmode == O_RDONLY)
	{
		SetHandleInformation(read_end, HANDLE_FLAG_INHERIT, 0);
	}
	else
	{
		SetHandleInformation(write_end, HANDLE_FLAG_INHERIT, 0);
	}

	int length = strlen(command) + 1;
	char *actual_command = (char *)malloc(sizeof(char) * (length + 11)); //"cmd.exe /C "
	memcpy(actual_command, "cmd.exe /C ", 11);
	memcpy(actual_command + 11, command, length);

	STARTUPINFOA SINFO;
	PROCESS_INFORMATION PINFO;
	memset(&SINFO, 0, sizeof(SINFO));
	SINFO.cb = sizeof(SINFO);
	memset(&PINFO, 0, sizeof(PINFO));
	SINFO.dwFlags |= STARTF_USESTDHANDLES;

	if (pmode == O_RDONLY)
	{
		SINFO.hStdOutput = write_end;
		SINFO.hStdError = write_end;
	}
	else
	{
		SINFO.hStdInput = read_end;
	}

	status = CreateProcessA(NULL, actual_command, NULL, NULL, TRUE, 0, NULL, NULL, &SINFO, &PINFO);

	if (pmode == O_RDONLY)
	{
		CloseHandle(write_end);
	}
	else
	{
		CloseHandle(read_end);
	}

	free(actual_command);

	if (status == 0)
	{
		map_win32_error_to_wlibc(GetLastError());
		CloseHandle(read_end);
		CloseHandle(write_end);
		return NULL;
	}

	int fd;
	if (pmode == O_RDONLY)
	{
		fd = register_to_fd_table(read_end, NULL, PIPE_HANDLE, O_RDONLY);
	}
	else
	{
		fd = register_to_fd_table(write_end, NULL, PIPE_HANDLE, O_WRONLY);
	}

	FILE *stream = create_stream(fd, _IOBUFFER_INTERNAL | _IOFBF | get_buf_mode(pmode), 4096);
	stream->phandle = PINFO.hProcess;

	return stream;
}