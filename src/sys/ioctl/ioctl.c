/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/fcntl.h>
#include <internal/error.h>
#include <sys/ioctl.h>
#include <stdarg.h>

int wlibc_ioctl(int fd, unsigned long request, va_list args)
{
	fdinfo info;
	get_fdinfo(fd, &info);

	if (info.type == INVALID_HANDLE)
	{
		errno = EBADF;
		return -1;
	}

	switch (request)
	{
	case TIOCGWINSZ:
	{
		struct winsize *window = va_arg(args, struct winsize *);
		CONSOLE_SCREEN_BUFFER_INFOEX csb = {.cbSize = sizeof(CONSOLE_SCREEN_BUFFER_INFOEX)};

		if (info.type != CONSOLE_HANDLE)
		{
			errno = ENOTTY;
			return -1;
		}

		if (window == NULL)
		{
			errno = EINVAL;
			return -1;
		}

		if (GetConsoleScreenBufferInfoEx(info.handle, &csb) == 0)
		{
			map_doserror_to_errno(GetLastError());
			return -1;
		}

		window->ws_col = csb.dwSize.X;
		window->ws_row = csb.dwSize.Y;
		window->ws_xpixel = 0;
		window->ws_ypixel = 0;

		return 0;
	}

	case TIOCSWINSZ:
	{
		struct winsize *window = va_arg(args, struct winsize *);
		COORD size;

		if (info.type != CONSOLE_HANDLE)
		{
			errno = ENOTTY;
			return -1;
		}

		if (window == NULL)
		{
			errno = EINVAL;
			return -1;
		}

		size.X = window->ws_col;
		size.Y = window->ws_row;

		if (SetConsoleScreenBufferSize(info.handle, size) == 0)
		{
			map_doserror_to_errno(GetLastError());
			return -1;
		}

		return 0;
	}

	case FIONREAD:
	{
		int *bytes = va_arg(args, int *);

		if (bytes == NULL)
		{
			errno = EINVAL;
			return -1;
		}

		if (info.type == PIPE_HANDLE)
		{
			NTSTATUS status;
			IO_STATUS_BLOCK io;
			FILE_PIPE_LOCAL_INFORMATION pipe_info;

			status = NtQueryInformationFile(info.handle, &io, &pipe_info, sizeof(FILE_PIPE_LOCAL_INFORMATION), FilePipeLocalInformation);

			if (status != STATUS_SUCCESS)
			{
				map_ntstatus_to_errno(status);
				return -1;
			}

			*bytes = (int)pipe_info.ReadDataAvailable;
			return 0;
		}

		if (info.type == CONSOLE_HANDLE)
		{
			DWORD events = 0;
			INPUT_RECORD record[16];

			if (PeekConsoleInputW(info.handle, record, 16, &events) == 0)
			{
				map_doserror_to_errno(GetLastError());
				return -1;
			}

			// Only count keyboard events.
			for (DWORD i = 0; i < events; ++i)
			{
				if (record[i].EventType == KEY_EVENT)
				{
					++(*bytes);
				}
			}

			return 0;
		}

		*bytes = 0;
		return 0;
	}

	case FIONWRITE:
	{
		int *bytes = va_arg(args, int *);

		if (bytes == NULL)
		{
			errno = EINVAL;
			return -1;
		}

		if (info.type == PIPE_HANDLE)
		{
			NTSTATUS status;
			IO_STATUS_BLOCK io;
			FILE_PIPE_LOCAL_INFORMATION pipe_info;

			status = NtQueryInformationFile(info.handle, &io, &pipe_info, sizeof(FILE_PIPE_LOCAL_INFORMATION), FilePipeLocalInformation);

			if (status != STATUS_SUCCESS)
			{
				map_ntstatus_to_errno(status);
				return -1;
			}

			*bytes = (int)(pipe_info.OutboundQuota - pipe_info.WriteQuotaAvailable);
			return 0;
		}

		*bytes = 0;
		return 0;
	}

	default:
		errno = EINVAL;
		return -1;
	}

}
