/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <wchar.h>
#include <stdlib.h>
#include <misc.h>
#include <fcntl_internal.h>
#include <dlfcn_internal.h>
#include <langinfo_internal.h>
#include <process_internal.h>
#include <signal_internal.h>

int main(int argc, char **argv);

__declspec(dllimport) int __cdecl _setmode(int fd, int mode);

int wmain(int argc, wchar_t **wargv)
{
	char **argv = NULL;
	if (argc)
	{
		argv = (char **)malloc(sizeof(char *) * (argc + 1)); // argv ends with NULL
		for (int i = 0; i < argc; i++)
		{
			argv[i] = wc_to_mb(wargv[i]);
		}
		argv[argc] = NULL;
	}

#ifdef WLIBC_BINARY_STD_STREAMS
	// #define O_BINARY 0x8000
	_setmode(0, 0x8000); // stdin
	_setmode(1, 0x8000); // stdout
	_setmode(2, 0x8000); // stderr
#endif

#ifdef WLIBC_POSIX_IO
	init_fd_table();
#endif
#ifdef WLIBC_DLFCN
	dlfcn_init();
#endif
#ifdef WLIBC_LANGINFO
	langinfo_init();
#endif
#ifdef WLIBC_PROCESS
	process_init();
#endif
#ifdef WLIBC_SIGNALS
	signal_init();
#endif

	int exit_status = main(argc, argv);

#ifdef WLIBC_POSIX_IO
	cleanup_fd_table();
#endif
#ifdef WLIBC_DLFCN
	dlfcn_cleanup();
#endif
#ifdef WLIBC_LANGINFO
	langinfo_cleanup();
#endif
#ifdef WLIBC_PROCESS
	process_cleanup();
#endif
#ifdef WLIBC_SIGNALS
	signal_cleanup();
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
