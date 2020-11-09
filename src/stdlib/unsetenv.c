/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT
*/

#include <stdlib-ext.h>
#include <stdlib.h>
#include <errno.h>

int wlibc_unsetenv(const char *name)
{
	errno_t err = _putenv_s(name, "");
	return err == 0 ? 0 : -1;
}

int wlibc_wunsetenv(const wchar_t *name)
{
	errno_t err = _wputenv_s(name, L"");
	return err == 0 ? 0 : -1;
}
