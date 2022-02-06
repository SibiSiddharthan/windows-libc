/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <dirent.h>
#include <string.h>
#include <wchar.h>

int wlibc_alphasort(const struct dirent **e1, const struct dirent **e2)
{
	return strcoll((*e1)->d_name, (*e2)->d_name);
}
