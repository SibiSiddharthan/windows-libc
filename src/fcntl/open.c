/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <fcntl.h>
#include <errno.h>
#include <wchar.h>
#include <string.h>
#include <windows.h>
#include <internal/fcntl.h>
#include <stdarg.h>
#include <internal/error.h>
#include <stdlib.h>
#include <internal/misc.h>
#include <sys/stat.h>

static DWORD determine_access_rights(const int oflags)
{
	DWORD flags = FILE_READ_DATA;
	if ((oflags & O_WRONLY) || (oflags & O_RDWR))
	{
		flags |= FILE_WRITE_DATA;
	}
	if (oflags & O_APPEND)
	{
		flags |= FILE_APPEND_DATA;
	}
	return flags;
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
	return -1ul; // unused
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
	DWORD access_rights = FILE_READ_ATTRIBUTES |
						  // chmod needs this as we change the read-only bit
						  FILE_WRITE_ATTRIBUTES |
						  // We don't technically need this as all users have BYPASS_TRAVERSE_CHECKING privelege, but just in case
						  FILE_TRAVERSE;
						  // chown
						  // READ_CONTROL| WRITE_DAC;
	DWORD share_rights = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
	DWORD create_how = 0;
	DWORD file_attributes = FILE_FLAG_BACKUP_SEMANTICS; // To also open a directory

	// access rights
	// If O_PATH is given file should not have read and write access
	if ((oflags & O_PATH) == 0)
	{
		access_rights |= determine_access_rights(oflags);
	}

	// create how
	create_how = determine_create_how(oflags);

	// file attributes
	file_attributes |= determine_file_attributes(oflags);

	// permissions
	file_attributes |= determine_permissions(perm);

	HANDLE file = CreateFile(wname, access_rights, share_rights, NULL, create_how, file_attributes, NULL);
	if (file == INVALID_HANDLE_VALUE)
	{
		map_win32_error_to_wlibc(GetLastError());
		return -1;
	}

	FILE_BASIC_INFO INFO;
	if (!GetFileInformationByHandleEx(file, FileBasicInfo, &INFO, sizeof(FILE_BASIC_INFO)))
	{
		map_win32_error_to_wlibc(GetLastError());
	}
	int fd;

	if (((oflags & O_NOFOLLOW) && ((oflags & O_PATH) == 0)) && (INFO.FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT))
	{
		// Close the handle if file is a symbolic link but only O_NOFOLLOW is specified. (Specify O_PATH also)
		CloseHandle(file);
		errno = ELOOP;
		return -1;
	}

	if ((oflags & O_DIRECTORY) && ((INFO.FileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0))
	{
		// Close the handle if file is not a directory and O_DIRECTORY is specified
		CloseHandle(file);
		errno = ENOTDIR;
		return -1;
	}

	if ((access_rights & (FILE_WRITE_DATA | FILE_APPEND_DATA)) && (INFO.FileAttributes & FILE_ATTRIBUTE_DIRECTORY))
	{
		// Close the handle if we request write access to a directory
		CloseHandle(file);
		errno = EISDIR;
		return -1;
	}

	fd = register_to_fd_table(file, wname, (INFO.FileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? DIRECTORY_HANDLE : FILE_HANDLE, oflags);
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
