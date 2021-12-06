/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_DIRENT_INTERNAL_H
#define WLIBC_DIRENT_INTERNAL_H

#include <sys/types.h>
#include <Windows.h>

typedef struct WLIBC_DIR
{
	unsigned int magic;
	int fd;
	void *buffer;
	size_t offset;
	size_t read_data;
	size_t received_data;
	CRITICAL_SECTION critical;
} DIR;

#define DIR_STREAM_MAGIC 0x1
#define VALIDATE_DIR_STREAM(stream, ret)                     \
	if (stream == NULL || stream->magic != DIR_STREAM_MAGIC) \
	{                                                        \
		errno = EBADF;                                      \
		return ret;                                          \
	}

#define LOCK_DIR_STREAM(stream)   EnterCriticalSection(&(stream->critical))
#define UNLOCK_DIR_STREAM(stream) LeaveCriticalSection(&(stream->critical))

#define DIRENT_DIR_BUFFER_SIZE 131072 // 128 KB. This allows a minimum of 250 entries.

#endif
