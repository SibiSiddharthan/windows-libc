#include <dlfcn.h>
#include <windows.h>
#include <wlibc_errors.h>

void *dlsym(void *handle, const char *symbol)
{
	void *ptr = GetProcAddress((HMODULE)handle, symbol);
	if (ptr == NULL)
	{
		_last_dlfcn_error = GetLastError();
		map_win32_error_to_wlibc(_last_dlfcn_error);
		return NULL;
	}
	return ptr;
}