#include <wlibc_errors.h>
#include <errno.h>
#include <windows.h>
//#include <stdio.h>

void map_win32_error_to_wlibc(unsigned long error)
{
	//printf("%lu\n", error);
	switch (error)
	{
	case ERROR_NOT_ENOUGH_MEMORY:
	case ERROR_OUTOFMEMORY:
		errno = ENOMEM;
		break;
	case ERROR_ACCESS_DENIED:
		errno = EACCES;
		break;
	case ERROR_PATH_NOT_FOUND:
		errno = ENOENT;
		break;
	case ERROR_TOO_MANY_OPEN_FILES:
		errno = EMFILE;
		break;
	case ERROR_DIRECTORY:
		errno = ENOTDIR;
		break;
	case ERROR_ALREADY_EXISTS:
		errno = EEXIST;
		break;
	default:
		break;
	};
}
