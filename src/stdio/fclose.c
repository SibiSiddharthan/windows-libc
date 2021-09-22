/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <internal/stdio.h>
#include <internal/fcntl.h>
#include <Windows.h>

int common_fflush(FILE *stream);
int wlibc_close(int fd);

int common_fclose(FILE *stream)
{
	int fd = stream->fd;

	common_fflush(stream);

	if ((stream->buf_mode & _IOBUFFER_INTERNAL) && (stream->buf_mode & _IOBUFFER_ALLOCATED))
	{
		free(stream->buffer);
	}

	// stream is freed here
	delete_stream(stream);

	return wlibc_close(fd);
}

int wlibc_fclose(FILE *stream)
{
	VALIDATE_FILE_STREAM(stream, -1);
	return common_fclose(stream);
}

int wlibc_fcloseall()
{
	cleanup_stdio();
	return 0;
}