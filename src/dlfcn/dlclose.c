#include <dlfcn.h>
#include <windows.h>

int wlibc_dlclose(void *handle)
{
	if (FreeLibrary((HMODULE)handle)) // success -> non zero
	{
		return 0;
	}
	else
	{
		return -1;
	}
}