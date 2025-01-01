/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <stdlib.h>
#include <errno.h>

/*
   The environment variables are managed with the environ table.
   Let's not mess with MSVC's environment handling by directly calling
   SetEnvironmentVariable().
*/

int wlibc_setenv(const char *restrict name, const char *restrict value, int overwrite)
{
	errno_t err;
	if (!getenv(name)) // Variable not found
	{
		err = _putenv_s(name, value);
		return err == 0 ? 0 : -1;
	}
	else
	{
		if (overwrite)
		{
			err = _putenv_s(name, value);
		}
		else
		{
			return 0;
		}
	}
	return err == 0 ? 0 : -1;
}

int wlibc_wsetenv(const wchar_t *restrict name, const wchar_t *restrict value, int overwrite)
{
	errno_t err;
	if (!_wgetenv(name)) // Variable not found
	{
		err = _wputenv_s(name, value);
		return err == 0 ? 0 : -1;
	}
	else
	{
		if (overwrite)
		{
			err = _wputenv_s(name, value);
		}
		else
		{
			return 0;
		}
	}
	return err == 0 ? 0 : -1;
}
