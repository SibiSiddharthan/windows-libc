/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT
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
