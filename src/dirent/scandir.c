/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/fcntl.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

// From readdir.c
struct dirent *do_readdir(DIR *dirstream, struct dirent *entry);

int wlibc_common_scandir(int dfd, const char *path, struct dirent ***entries, int (*selector)(const struct dirent *),
						 int (*cmp)(const struct dirent **, const struct dirent **))
{
	VALIDATE_PATH(path, ENOENT, -1);

	HANDLE handle = just_open(dfd, path, FILE_READ_ATTRIBUTES | FILE_TRAVERSE | FILE_LIST_DIRECTORY | SYNCHRONIZE,
							  FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT);
	if (handle == NULL)
	{
		// errno wil be set by just_open.
		return -1;
	}

	int fd = register_to_fd_table(handle, DIRECTORY_HANDLE, O_RDONLY | O_CLOEXEC | O_DIRECTORY);
	DIR *dirstream = wlibc_fdopendir(fd);

	if (dirstream == NULL)
	{
		return -1;
	}

	int count = 0;
	int allocated = 16;
	struct dirent entry;

	*entries = (struct dirent **)malloc(sizeof(struct dirent *) * 16);

	while (do_readdir(dirstream, &entry) != NULL)
	{
		if (selector)
		{
			// Skip the entry if the selector function return 0.
			if (selector(&entry) == 0)
			{
				continue;
			}
		}

		// Double the allocated entry list.
		if (count == allocated)
		{
			*entries = realloc(*entries, sizeof(struct dirent *) * 32);
			allocated *= 2;
		}

		(*entries)[count] = (struct dirent *)malloc(sizeof(struct dirent));
		memcpy((*entries)[count], &entry, sizeof(struct dirent));
		++count;
	}

	if (cmp)
	{
		qsort(*entries, count, sizeof(struct dirent *), (int (*)(const void *, const void *))cmp);
	}

	wlibc_closedir(dirstream);

	return count;
}
