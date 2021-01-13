/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
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
