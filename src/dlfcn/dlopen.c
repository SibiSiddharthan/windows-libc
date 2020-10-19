#include <dlfcn.h>
#include <windows.h>
#include <wchar.h>
#include <wlibc_errors.h>
#include <misc.h>

void *dlopen(const char *filename, int flags /*unused*/)
{
	if (filename == NULL)
	{
		return NULL;
	}

	wchar_t *wfilename = mb_to_wc(filename);

	HMODULE module = LoadLibrary(wfilename);
	if (module == NULL)
	{
		_last_dlfcn_error = GetLastError();
		map_win32_error_to_wlibc(_last_dlfcn_error);
		return NULL;
	}

	free(wfilename);
	return (void *)module;
}