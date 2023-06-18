/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/dirent.h>
#include <internal/error.h>
#include <internal/fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>

static DIR *initialize_dirstream(int fd)
{
	DIR *dirstream = (DIR *)RtlAllocateHeap(NtCurrentProcessHeap(), HEAP_ZERO_MEMORY, sizeof(DIR) + DIRENT_DIR_BUFFER_SIZE);

	if (dirstream == NULL)
	{
		errno = ENOMEM;
		return NULL;
	}

	dirstream->magic = DIR_STREAM_MAGIC;
	dirstream->fd = fd;
	dirstream->buffer = (void *)((char *)dirstream + sizeof(DIR));

	RtlInitializeCriticalSection(&(dirstream->critical));

	return dirstream;
}

DIR *wlibc_opendir(const char *path)
{
	VALIDATE_PATH(path, ENOENT, NULL);

	HANDLE handle = just_open(AT_FDCWD, path, FILE_READ_ATTRIBUTES | FILE_TRAVERSE | FILE_LIST_DIRECTORY | SYNCHRONIZE,
							  FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT);
	if (handle == INVALID_HANDLE_VALUE)
	{
		// errno wil be set by just_open
		return NULL;
	}

	int fd = register_to_fd_table(handle, DIRECTORY_HANDLE, O_RDONLY | O_CLOEXEC | O_DIRECTORY);

	DIR *dirstream = initialize_dirstream(fd);

	// Close the file descriptor if we fail to initialize the stream.
	if (dirstream == NULL)
	{
		close_fd(fd);
	}

	return dirstream;
}

DIR *wlibc_fdopendir(int fd)
{
	handle_t _type = get_fd_type(fd);
	if (_type != DIRECTORY_HANDLE || _type == INVALID_HANDLE)
	{
		errno = (_type == INVALID_HANDLE ? EBADF : ENOTDIR);
		return NULL;
	}

	DIR *dirstream = initialize_dirstream(fd);

	return dirstream;
}
