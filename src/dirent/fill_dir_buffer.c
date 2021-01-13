/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <dirent.h>
#include <windows.h>
#include <wchar.h>

// Fill the data buffer in DIR structure for faster access
void fill_dir_buffer(DIR *dirp)
{
	WIN32_FIND_DATA find_file = dirp->data[dirp->size - 1];
	for (int i = dirp->size; i < dirp->buffer_length; i++)
	{
		if (FindNextFile(dirp->d_handle, &find_file))
		{
			dirp->data[i] = find_file;
			dirp->size++;
		}
		else
		{
			break;
		}
	}
}
