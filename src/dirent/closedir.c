/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <internal/dirent.h>
#include <internal/fcntl.h>
#include <dirent.h>
#include <stdlib.h>

int wlibc_closedir(DIR *dirstream)
{
	VALIDATE_DIR_STREAM(dirstream, -1);

	if (!close_fd(dirstream->fd))
	{
		// Free the memory of DIR and it's components
		free(dirstream->buffer);
		RtlDeleteCriticalSection(&(dirstream->critical));
		free(dirstream);
		return 0;
	}
	else
	{
		return -1;
	}
}
