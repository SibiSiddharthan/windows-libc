/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <internal/dirent.h>
#include <internal/fcntl.h>
#include <dirent.h>

int wlibc_closedir(DIR *dirstream)
{
	VALIDATE_DIR_STREAM(dirstream, -1);

	if (!close_fd(dirstream->fd))
	{
		// Free the memory of DIR.
		RtlDeleteCriticalSection(&(dirstream->critical));
		if (RtlFreeHeap(NtCurrentProcessHeap(), 0, dirstream) == FALSE)
		{
			return -1;
		}

		return 0;
	}
	else
	{
		return -1;
	}
}
