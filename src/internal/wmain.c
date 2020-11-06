/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT
*/

#include <wchar.h>
#include <stdlib.h>
#include <string.h>
#include <misc.h>
#include <fcntl_internal.h>
#include <dlfcn_internal.h>

int main(int argc, char **argv);

int wmain(int argc, wchar_t **wargv)
{
	char **argv = NULL;
	if (argc)
	{
		argv = (char **)malloc(sizeof(char *) * argc);
		for (int i = 0; i < argc; i++)
		{
			argv[i] = wc_to_mb(wargv[i]);
		}
	}

#ifdef WLIBC_POSIX_IO
	init_fd_table();
#endif
#ifdef WLIBC_DLFCN
	dlfcn_init();
#endif

	int exit_status = main(argc, argv);

#ifdef WLIBC_POSIX_IO
	cleanup_fd_table();
#endif
#ifdef WLIBC_DLFCN
	dlfcn_cleanup();
#endif

	if (argc)
	{
		for (int i = 0; i < argc; i++)
		{
			free(argv[i]);
		}
		free(argv);
	}

	return exit_status;
}