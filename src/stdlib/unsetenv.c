/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

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
