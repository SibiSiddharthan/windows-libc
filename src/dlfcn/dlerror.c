#include <dlfcn.h>
#include <windows.h>
#include <wlibc_errors.h>

char *dlerror()
{
	if (_last_dlfcn_error == ERROR_SUCCESS)
	{
		return NULL;
	}

	//Call the ANSI version explicitly as we are always returning char*
	DWORD length = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
								  NULL, _last_dlfcn_error, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), _dlfcn_error_message, 65535, NULL);

	_last_dlfcn_error = ERROR_SUCCESS;

	if (length == 0)
	{
		map_win32_error_to_wlibc(GetLastError());
		return NULL;
	}

	return _dlfcn_error_message;
}