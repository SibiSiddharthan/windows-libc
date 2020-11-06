/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT
*/

#include <stdio-hooks.h>
#include <string.h>
#include <Windows.h>
#include <errno.h>
#include <io.h>
#include <fcntl_internal.h>
#include <fcntl.h>
#include <misc.h>
#include <stdlib.h>

static int parse_mode(const char *mode)
{
	int flags = 0;
	int length = strlen(mode);
	for (int i = 0; i < length; i++)
	{
		switch (mode[i])
		{
		case 'r':
			flags |= O_RDONLY;
			break;
		case 'w':
			flags |= O_WRONLY | O_CREAT | O_TRUNC;
			break;
		case '+':
			flags |= O_RDWR;
			break;
		case 'a':
			flags |= O_APPEND;
			break;
		case 'b':
			flags |= O_BINARY;
			break;
		case 't':
			flags |= O_TEXT;
			break;
		default:
			break;
		}
	}

	return flags;
}

FILE *wlibc_fopen(const char *filename, const char *mode)
{
	DWORD attributes = GetFileAttributesA(filename);
	char *filename_proper = NULL;
	// Redirect /dev/null to NUL
	if (strcmp(filename, "/dev/null") == 0)
	{
		filename_proper = "NUL";
	}

	// Do not open a directory
	else if (attributes == FILE_ATTRIBUTE_DIRECTORY || (attributes == (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)))
	{
		errno = EISDIR;
		return NULL;
	}

	else
	{
		filename_proper = (char *)filename;
	}

	FILE *_FILE = _fsopen(filename_proper, mode, _SH_DENYNO);
	if (_FILE == NULL)
	{
		return NULL;
	}
	HANDLE file_handle = (HANDLE)_get_osfhandle(_fileno(_FILE));
	wchar_t *wpath = mb_to_wc(filename_proper);
	int flags = parse_mode(mode);
	register_to_fd_table(file_handle, wpath, NORMAL_FILE_ACTIVE, flags);
	free(wpath);
	return _FILE;
}

FILE *wlibc_fdopen(int fd, const char *mode)
{
	// We have the fd information
	if (!validate_active_ffd(fd))
	{
		return NULL;
	}

	// We opened the handle with CreateFile (or one of std* streams), the underlying OS handle is not changed here.
	HANDLE file = get_fd_handle(fd);
	int crt_fd = _open_osfhandle((intptr_t)file, 0);
	FILE *_FILE = _fdopen(crt_fd, mode);
	int flags = parse_mode(mode);
	if (flags & O_APPEND)
	{
		add_fd_flags(fd, O_APPEND);
	}
	return _FILE;
}

#undef freopen
FILE *wlibc_freopen(const char *filename, const char *mode, FILE *old_stream)
{
	DWORD attributes = GetFileAttributesA(filename);
	char *filename_proper = NULL;
	// Redirect /dev/null to NUL
	if (strcmp(filename, "/dev/null") == 0)
	{
		filename_proper = "NUL";
	}

	// Do not open a directory
	else if (attributes == FILE_ATTRIBUTE_DIRECTORY || (attributes == (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)))
	{
		errno = EISDIR;
		return NULL;
	}

	else
	{
		filename_proper = (char *)filename;
	}

	HANDLE old_handle = (HANDLE)_get_osfhandle(_fileno(old_stream));
	int fd = get_fd(old_handle);

	FILE *new_stream = freopen(filename_proper, mode, old_stream);
	if (new_stream == NULL)
	{
		return NULL;
	}
	HANDLE new_handle = (HANDLE)_get_osfhandle(_fileno(new_stream));

	int flags = parse_mode(mode);
	wchar_t *wpath = mb_to_wc(filename_proper);
	set_fd_handle(fd, new_handle);
	set_fd_type(fd, NORMAL_FILE_ACTIVE);
	set_fd_flags(fd, flags);
	set_fd_path(fd, wpath);
	free(wpath);
	return new_stream;
}

int wlibc_fileno(FILE *stream)
{
	// Return our file descriptor
	HANDLE file_handle = (HANDLE)_get_osfhandle(_fileno(stream));
	return get_fd(file_handle);
}

#undef fclose
int wlibc_fclose(FILE *stream)
{
	int status = -1;
	if (stream)
	{
		HANDLE file_handle = (HANDLE)_get_osfhandle(_fileno(stream));
		status = fclose(stream); // Underlying OS handle is closed here
		unregister_from_fd_table(file_handle);
	}

	return status;
}