/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT
*/

#include <fcntl.h>
#include <errno.h>
#include <wchar.h>
#include <string.h>
#include <windows.h>
#include <fcntl_internal.h>
#include <stdarg.h>
#include <wlibc_errors.h>
#include <stdlib.h>
#include <misc.h>

static DWORD determine_access_rights(const int oflags)
{
	if (oflags & O_RDONLY)
	{
		return GENERIC_READ;
	}
	else if (oflags & O_WRONLY)
	{
		return GENERIC_WRITE;
	}
	else if ((oflags & O_RDWR) || (oflags & (O_RDONLY | O_WRONLY)))
	{
		return GENERIC_READ | GENERIC_WRITE;
	}
	else
	{
		return -1ul;
	}
}

static DWORD determine_create_how(const int oflags)
{
	DWORD main_flags = oflags & (O_CREAT | O_EXCL | O_TRUNC);
	switch (main_flags)
	{
	case 0x0:
	case O_EXCL:
		return OPEN_EXISTING;
	case O_CREAT:
		return OPEN_ALWAYS;
	case O_TRUNC:
	case O_TRUNC | O_EXCL:
		return TRUNCATE_EXISTING;
	case O_CREAT | O_EXCL:
	case O_CREAT | O_TRUNC | O_EXCL:
		return CREATE_NEW;
	case O_CREAT | O_TRUNC:
		return CREATE_ALWAYS;
	}
	return -1ul;
}

static DWORD determine_file_attributes(const int oflags)
{
	DWORD attributes = 0;
	if (oflags & O_TMPFILE)
	{
		attributes |= FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE;
	}
	if (oflags & O_DIRECT)
	{
		attributes |= FILE_FLAG_NO_BUFFERING;
	}
	if (oflags & O_SYNC)
	{
		attributes |= FILE_FLAG_WRITE_THROUGH;
	}
	if (oflags & O_RANDOM)
	{
		attributes |= FILE_FLAG_RANDOM_ACCESS;
	}
	if (oflags & O_SEQUENTIAL)
	{
		attributes |= FILE_FLAG_SEQUENTIAL_SCAN;
	}
	if (oflags & O_NONBLOCK)
	{
		//	attributes |= FILE_FLAG_OVERLAPPED; TODO
	}
	if (oflags & O_NOFOLLOW)
	{
		attributes |= FILE_FLAG_OPEN_REPARSE_POINT;
	}

	return attributes;
}

static DWORD determine_permissions(const mode_t perm)
{
	if (perm == 0)
	{
		return 0;
	}
	if (perm & S_IWRITE)
	{
		return 0;
	}
	return FILE_ATTRIBUTE_READONLY;
}

int common_open(const wchar_t *wname, const int oflags, const mode_t perm)
{
	DWORD access_rights = 0;
	DWORD share_rights = FILE_SHARE_READ | FILE_SHARE_WRITE;
	DWORD create_how = 0;
	DWORD file_attributes = FILE_FLAG_BACKUP_SEMANTICS; // To also open a directory

	// access rights
	access_rights = determine_access_rights(oflags);
	if (access_rights == -1ul)
	{
		errno = EINVAL;
		return -1;
	}

	// create how
	create_how = determine_create_how(oflags);
	if (create_how == -1ul)
	{
		errno = EINVAL;
		return -1;
	}

	// file attributes
	file_attributes |= determine_file_attributes(oflags);
	if (file_attributes == FILE_FLAG_BACKUP_SEMANTICS)
	{
		file_attributes |= FILE_ATTRIBUTE_NORMAL;
	}

	// permissions
	file_attributes |= determine_permissions(perm);

	HANDLE file = CreateFile(wname, access_rights, share_rights, NULL, create_how, file_attributes, NULL);
	if (file == INVALID_HANDLE_VALUE)
	{
		map_win32_error_to_wlibc(GetLastError());
		return -1;
	}

	BY_HANDLE_FILE_INFORMATION INFO;
	GetFileInformationByHandle(file, &INFO);
	int fd;

	if ((oflags & O_NOFOLLOW) && (INFO.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT))
	{
		// Close the handle if file is symbolic link as O_NOFOLLOW dictates
		CloseHandle(file);
		errno = ELOOP;
		return -1;
	}

	if ((oflags & O_DIRECTORY) && ((INFO.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0))
	{
		// Close the handle if file is not a directory
		CloseHandle(file);
		errno = ENOTDIR;
		return -1;
	}

	if ((access_rights & GENERIC_WRITE) && (INFO.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
	{
		// Close the handle if we request write access to a directory
		CloseHandle(file);
		errno = EISDIR;
		return -1;
	}

	if (INFO.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	{
		// Use an empty handle for directories. This will later be filled up the fdopendir
		CloseHandle(file);
		fd = register_to_fd_table(NULL, wname, DIRECTORY_INACTIVE, 0);
		return fd;
	}

	if (oflags & O_PATH)
	{
		CloseHandle(file);
		fd = register_to_fd_table(NULL, wname, NORMAL_FILE_INACTIVE, 0);
		return fd;
	}

	fd = register_to_fd_table(file, wname, NORMAL_FILE_ACTIVE, oflags);
	return fd;
}

int wlibc_open(const char *name, const int oflags, va_list perm_args)
{
	if (name == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	mode_t perm;
	if (oflags & O_CREAT)
	{
		perm = va_arg(perm_args, mode_t);
	}
	else
	{
		perm = 0;
	}

	int fd;

	if (strcmp(name, "/dev/null") == 0)
	{
		wchar_t *wname = L"NUL";
		fd = common_open(wname, oflags, perm);
	}
	else if (strcmp(name, "/dev/tty") == 0)
	{
		wchar_t *wname = L"CON";
		fd = common_open(wname, oflags, perm);
	}
	else
	{
		wchar_t *wname = mb_to_wc(name);
		fd = common_open(wname, oflags, perm);
		free(wname);
	}
	return fd;
}

int wlibc_wopen(const wchar_t *wname, const int oflags, va_list perm_args)
{
	if (wname == NULL)
	{
		errno = ENOENT;
		return -1;
	}

	mode_t perm;
	if (oflags & O_CREAT)
	{
		perm = va_arg(perm_args, mode_t);
	}
	else
	{
		perm = 0;
	}

	if (wcscmp(wname, L"/dev/null") == 0)
	{
		wchar_t *wname = L"NUL";
		return common_open(wname, oflags, perm);
	}
	else if (wcscmp(wname, L"/dev/tty") == 0)
	{
		wchar_t *wname = L"CON";
		return common_open(wname, oflags, perm);
	}
	else
	{
		return common_open(wname, oflags, perm);
	}
}