/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT
*/

#include <dirent.h>
#include <string.h>
#include <wchar.h>

int wlibc_alphasort(const struct dirent **e1, const struct dirent **e2)
{
	return strcoll((*e1)->d_name, (*e2)->d_name);
}

int wlibc_walphasort(const struct wdirent **e1, const struct wdirent **e2)
{
	return wcscoll((*e1)->d_name, (*e2)->d_name);
}