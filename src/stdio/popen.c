/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <internal/fcntl.h>
#include <internal/spawn.h>
#include <internal/stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

FILE *wlibc_popen(const char *restrict command, const char *restrict mode)
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
		map_doserror_to_errno(GetLastError());
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

	size_t length = strlen(command) + 1;
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
		map_doserror_to_errno(GetLastError());
		CloseHandle(read_end);
		CloseHandle(write_end);
		return NULL;
	}

	int fd;
	if (pmode == O_RDONLY)
	{
		fd = register_to_fd_table(read_end, PIPE_HANDLE, O_RDONLY);
	}
	else
	{
		fd = register_to_fd_table(write_end, PIPE_HANDLE, O_WRONLY);
	}

	FILE *stream = create_stream(fd, _IOBUFFER_INTERNAL | _IOFBF | get_buf_mode(pmode), 4096);
	stream->phandle = PINFO.hProcess;

	return stream;
}
