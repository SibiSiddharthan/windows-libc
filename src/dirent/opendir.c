/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/dirent.h>
#include <dirent.h>
#include <wchar.h>
#include <stdlib.h>
#include <string.h>
#include <internal/error.h>
#include <errno.h>
#include <internal/fcntl.h>
#include <internal/misc.h>
#include <fcntl.h>

static void initialize_dirstream(DIR **dirstream, int fd)
{
	*dirstream = (DIR *)malloc(sizeof(DIR));
	(*dirstream)->magic = DIR_STREAM_MAGIC;
	(*dirstream)->fd = fd;
	(*dirstream)->buffer = malloc(DIRENT_DIR_BUFFER_SIZE);
	(*dirstream)->offset = 0;
	(*dirstream)->read_data = 0;
	(*dirstream)->received_data = 0;
	(*dirstream)->info = (struct dirent *)malloc(sizeof(struct dirent));
	InitializeCriticalSection(&((*dirstream)->critical));
}

DIR *wlibc_opendir(const char *path)
{
	VALIDATE_PATH(path, ENOENT, NULL);

	wchar_t *u16_ntpath = get_absolute_ntpath(AT_FDCWD, path);
	if (u16_ntpath == NULL)
	{
		errno = ENOENT;
		return NULL;
	}

	HANDLE handle = just_open(u16_ntpath, FILE_READ_ATTRIBUTES | FILE_TRAVERSE | FILE_LIST_DIRECTORY | SYNCHRONIZE, 0, FILE_OPEN,
							  FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT);
	if (handle == INVALID_HANDLE_VALUE)
	{
		// errno wil be set by just_open
		free(u16_ntpath);
		return NULL;
	}

	int fd = register_to_fd_table(handle, u16_ntpath + 4, DIRECTORY_HANDLE, O_RDONLY | O_CLOEXEC | O_DIRECTORY);
	free(u16_ntpath);

	DIR *dirstream = NULL;
	initialize_dirstream(&dirstream, fd);

	return dirstream;
}

DIR *wlibc_fdopendir(int fd)
{
	enum handle_type _type = get_fd_type(fd);
	if (_type != DIRECTORY_HANDLE || _type == INVALID_HANDLE)
	{
		errno = (_type == INVALID_HANDLE ? EBADF : ENOTDIR);
		return NULL;
	}

	DIR *dirstream = NULL;
	initialize_dirstream(&dirstream, fd);

	return dirstream;
}
