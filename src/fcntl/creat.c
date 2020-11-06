/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT
*/

#include <fcntl.h>
#include <wchar.h>

int wlibc_creat(const char *name, const mode_t mode)
{
	return open(name, O_WRONLY | O_CREAT | O_TRUNC, mode);
}

int wlibc_wcreat(const wchar_t *wname, const mode_t mode)
{
	return wopen(wname, O_WRONLY | O_CREAT | O_TRUNC, mode);
}