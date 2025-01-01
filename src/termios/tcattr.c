/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/fcntl.h>
#include <internal/validate.h>
#include <internal/error.h>
#include <termios.h>
#include <Windows.h>

int wlibc_tcgetattr(int fd, struct termios *termio)
{
	BOOL status;
	DWORD mode;
	fdinfo info;

	VALIDATE_PTR(termio, EINVAL, -1);

	get_fdinfo(fd, &info);

	if (info.type != CONSOLE_HANDLE)
	{
		errno = EBADF;
		return -1;
	}

	status = GetConsoleMode(info.handle, &mode);

	if (status == 0)
	{
		map_doserror_to_errno(GetLastError());
		return -1;
	}

	termio->c_iflag = 0;
	termio->c_oflag = 0;
	termio->c_cflag = 0;
	termio->c_lflag = 0;
	termio->c_ispeed = BAUD_38400;
	termio->c_ospeed = BAUD_38400;

	if (mode & ENABLE_PROCESSED_INPUT)
	{
		termio->c_iflag |= IGNBRK;
	}
	if (mode & ENABLE_PROCESSED_OUTPUT)
	{
		termio->c_oflag |= OPOST;
	}
	if (mode & ENABLE_ECHO_INPUT)
	{
		termio->c_lflag |= ECHO;
	}
	if (mode & ENABLE_LINE_INPUT)
	{
		termio->c_lflag |= ICANON;
	}
	if (mode & ENABLE_VIRTUAL_TERMINAL_INPUT)
	{
		termio->c_lflag |= IEXTEN;
	}

	return 0;
}

int wlibc_tcsetattr(int fd, int action, const struct termios *termio)
{
	BOOL status;
	DWORD mode;
	fdinfo info;

	VALIDATE_PTR(termio, EINVAL, -1);

	if (action < TCSANOW || action > TCSAFLUSH)
	{
		errno = EINVAL;
		return -1;
	}

	get_fdinfo(fd, &info);

	if (info.type != CONSOLE_HANDLE)
	{
		errno = EBADF;
		return -1;
	}

	status = GetConsoleMode(info.handle, &mode);

	if (status == 0)
	{
		map_doserror_to_errno(GetLastError());
		return -1;
	}

	if (termio->c_iflag & IGNBRK)
	{
		mode |= ENABLE_PROCESSED_INPUT;
	}
	else
	{
		mode &= ~ENABLE_PROCESSED_INPUT;
	}

	if (termio->c_oflag & OPOST)
	{
		mode |= ENABLE_PROCESSED_OUTPUT;
	}
	else
	{
		mode &= ~ENABLE_PROCESSED_OUTPUT;
	}

	if (termio->c_lflag & ECHO)
	{
		mode |= ENABLE_ECHO_INPUT;
	}
	else
	{
		mode &= ~ENABLE_ECHO_INPUT;
	}

	if (termio->c_lflag & ICANON)
	{
		mode |= ENABLE_LINE_INPUT;
	}
	else
	{
		mode &= ~ENABLE_LINE_INPUT;
	}

	if (termio->c_lflag & IEXTEN)
	{
		mode |= ENABLE_VIRTUAL_TERMINAL_INPUT;
	}
	else
	{
		mode &= ~ENABLE_VIRTUAL_TERMINAL_INPUT;
	}

	status = SetConsoleMode(info.handle, mode);

	if (status == 0)
	{
		map_doserror_to_errno(GetLastError());
		return -1;
	}

	return 0;
}
