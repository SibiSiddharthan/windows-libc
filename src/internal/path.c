/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <internal/fcntl.h>
#include <internal/path.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

// temp.c
char *wlibc_tmpdir(void);

static int path_components_size = 16;

typedef struct
{
	int start;  // starting offset
	int length; // length of component in bytes
} path_component;

static path_component *add_component(path_component *restrict components, int *restrict index, int start, int length)
{
	if (components == NULL)
	{
		components = (path_component *)RtlAllocateHeap(NtCurrentProcessHeap(), 0, path_components_size * sizeof(path_component));

		if (components == NULL)
		{
			errno = ENOMEM;
			return NULL;
		}
	}

	if (*index == path_components_size)
	{
		void *temp = RtlReAllocateHeap(NtCurrentProcessHeap(), 0, components, path_components_size * 2 * sizeof(path_component));

		if (temp == NULL)
		{
			errno = ENOMEM;
			return NULL;
		}

		components = (path_component *)temp;
		path_components_size *= 2;
	}

	components[*index].start = start;
	components[*index].length = length;
	(*index)++;

	return components;
}

typedef struct _nt_device
{
	uint16_t length;  // Length in bytes
	wchar_t name[26]; // Name of the device eg. "\Device\HarddiskVolume1"
} nt_device;

// All available volumes A - Z
nt_device devices[32] = {0};

nt_device *dos_device_to_nt_device(char volume)
{
	NTSTATUS status;
	UNICODE_STRING path, realpath;
	OBJECT_ATTRIBUTES object;
	HANDLE handle;

	WCHAR path_buffer[] = L"\\GLOBAL??\\$:"; // '$' will be replaced by the drive letter

	path.Length = 24;
	path.MaximumLength = 26;
	path.Buffer = path_buffer;

	if (volume < 'A' || volume > 'Z')
	{
		return NULL;
	}

	// If length is '-1', it means the volume does not exist.
	if (devices[volume - 'A'].length != (uint16_t)-1)
	{
		if (devices[volume - 'A'].length != 0)
		{
			// In cache
			return &devices[volume - 'A'];
		}
		else
		{
			// Zero extension for UTF16-LE(Little Endian) works here
			path_buffer[10] = (WCHAR)volume;
			InitializeObjectAttributes(&object, &path, OBJ_CASE_INSENSITIVE, NULL, NULL);
			status = NtOpenSymbolicLinkObject(&handle, SYMBOLIC_LINK_QUERY, &object);
			if (status != STATUS_SUCCESS)
			{
				// Mark the volume as non existent.
				devices[volume - 'A'].length = (uint16_t)-1;
				return NULL;
			}

			// Use the cache structures buffer itself
			realpath.Buffer = devices[volume - 'A'].name;
			realpath.Length = 0;
			realpath.MaximumLength = 26 * sizeof(WCHAR);

			status = NtQuerySymbolicLinkObject(handle, &realpath, NULL);
			NtClose(handle);
			// This should not fail.
			if (status != STATUS_SUCCESS)
			{
				// Mark the volume as non existent.
				devices[volume - 'A'].length = (uint16_t)-1;
				return NULL;
			}

			devices[volume - 'A'].length = realpath.Length;
			return &devices[volume - 'A'];
		}
	}

	return NULL;
}

typedef struct _dos_device
{
	dev_t device_number;
	char label;
} dos_device;

// sizeof(dos_device) is equal to 8, just return by value.
char nt_device_to_dos_device(const UNICODE_STRING *device)
{
	NTSTATUS status;
	IO_STATUS_BLOCK io;
	HANDLE mountmgr_handle;
	MOUNTMGR_TARGET_NAME *device_name;
	MOUNTMGR_VOLUME_PATHS *dos_name;
	char device_name_buffer[64] = {0};
	char dos_name_buffer[16] = {0};
	char label;

	mountmgr_handle = open_mountmgr();

	device_name = (PMOUNTMGR_TARGET_NAME)device_name_buffer;
	memcpy(device_name->DeviceName, device->Buffer, device->Length);
	device_name->DeviceNameLength = device->Length;

	dos_name = (PMOUNTMGR_VOLUME_PATHS)dos_name_buffer;

	status = NtDeviceIoControlFile(mountmgr_handle, NULL, NULL, NULL, &io, IOCTL_MOUNTMGR_QUERY_DOS_VOLUME_PATHS, device_name,
								   sizeof(device_name_buffer), dos_name, sizeof(dos_name_buffer));
	NtClose(mountmgr_handle);

	if (status == STATUS_SUCCESS)
	{
		label = (char)dos_name->MultiSz[0];
		// Succeed only if it is a drive letter.
		if (label >= 'A' && label <= 'Z')
		{
			return label;
		}
	}

	return '\0';
}

typedef struct _fd_path
{
	int fd;
	unsigned int sequence;
	UNICODE_STRING *nt_path;
} fd_path;

#define FD_PATH_CACHE_SLOTS 128

fd_path fd_path_cache[FD_PATH_CACHE_SLOTS] = {{-1, 0, NULL}};

UNICODE_STRING *check_fd_path_cache(int fd, unsigned int sequence)
{
	int index = sequence & (FD_PATH_CACHE_SLOTS - 1);

	// In the cache.
	if (sequence == fd_path_cache[index].sequence && fd == fd_path_cache[index].fd)
	{
		return fd_path_cache[index].nt_path;
	}

	// Not in cache.
	return NULL;
}

void update_fd_path_cache(int fd, int sequence, PUNICODE_STRING nt_path)
{
	int index = sequence & (FD_PATH_CACHE_SLOTS - 1);
	fd_path_cache[index].sequence = sequence;
	fd_path_cache[index].fd = fd;
	if (fd_path_cache[index].nt_path != NULL)
	{
		// previosly used slot, free the memory.
		RtlFreeHeap(NtCurrentProcessHeap(), 0, fd_path_cache[index].nt_path);
	}
	fd_path_cache[index].nt_path = nt_path;
}

UNICODE_STRING *get_handle_ntpath(HANDLE handle)
{
	NTSTATUS status;
	ULONG length;
	UNICODE_STRING *path = NULL;

	path = RtlAllocateHeap(NtCurrentProcessHeap(), 0, 1024);
	if (path == NULL)
	{
		errno = ENOMEM;
		return NULL;
	}

	status = NtQueryObject(handle, ObjectNameInformation, path, 1024, &length);
	if (status == STATUS_BUFFER_OVERFLOW)
	{
		path = RtlReAllocateHeap(NtCurrentProcessHeap(), 0, path, length);
		if (path == NULL)
		{
			errno = ENOMEM;
			return NULL;
		}

		status = NtQueryObject(handle, ObjectNameInformation, path, length, &length);
	}

	// If we have an open handle, this should not fail.
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		RtlFreeHeap(NtCurrentProcessHeap(), 0, path);
		return NULL;
	}

	return path;
}

// NOTE: The returned pointer should not be freed.
UNICODE_STRING *get_fd_ntpath_internal(int fd)
{
	fdinfo info;
	UNICODE_STRING *path = NULL;

	get_fdinfo(fd, &info);

	if (info.type == INVALID_HANDLE)
	{
		return NULL;
	}

	// Check the cache first.
	path = check_fd_path_cache(fd, info.sequence);
	if (path != NULL)
	{
		return path;
	}

	// Not in cache do the lookup.
	path = get_handle_ntpath(info.handle);
	if (path != NULL)
	{
		// Update the cache.
		update_fd_path_cache(fd, info.sequence, path);
	}

	return path;
}

UNICODE_STRING *get_absolute_ntpath2(int dirfd, const char *path, handle_t *type)
{
	NTSTATUS status;
	UTF8_STRING u8_path;
	UNICODE_STRING u16_path, u16_rootdir = {0};

	// User should free the allocated memory.
	UNICODE_STRING *u16_ntpath = NULL;
	USHORT required_size = 0;

	nt_device *device = NULL;
	// malloc'ed resources
	path_component *components = NULL;
	char *rootdir_buffer = NULL;
	WCHAR *ntpath_buffer = NULL;

	bool path_is_absolute = false;
	bool cygwin_path = false;
	bool rootdir_has_trailing_slash = false;
	bool temp_path_requested = false;

	handle_t unused;

	if (type == NULL)
	{
		type = &unused;
	}

	// For all these predefined devices we set MaximumLength to be same as Length (omitting the terminating NULL).
	// This enables us to use the static storage an avoids needless memory allocation.

	// ROOT
	if (IS_ROOT_PATH(path))
	{
		// We should be able to stat root as many programs need this.
		// Here the root will refer to the current drive of the process.

		// UTF-16LE char truncation.
		device = dos_device_to_nt_device((char)(NtCurrentPeb()->ProcessParameters->CurrentDirectory.DosPath.Buffer[0]));
		u16_ntpath = (UNICODE_STRING *)RtlAllocateHeap(NtCurrentProcessHeap(), 0,
													   sizeof(UNICODE_STRING) + device->length + 2 * sizeof(WCHAR)); // '\\', '\0'

		if (u16_ntpath == NULL)
		{
			errno = ENOMEM;
			return NULL;
		}

		u16_ntpath->Buffer = (WCHAR *)((char *)u16_ntpath + sizeof(UNICODE_STRING));
		u16_ntpath->Length = device->length + sizeof(WCHAR);
		u16_ntpath->MaximumLength = u16_ntpath->Length + sizeof(WCHAR);

		memcpy(u16_ntpath->Buffer, device->name, device->length);
		memcpy((char *)u16_ntpath->Buffer + device->length, L"\\\0", 2 * sizeof(WCHAR));

		return u16_ntpath;
	}

	// POSIX devices
	if (strncmp(path, "/dev/", 5) == 0)
	{
		if (strncmp(path + 5, "null", 5) == 0)
		{
			u16_ntpath = (UNICODE_STRING *)RtlAllocateHeap(NtCurrentProcessHeap(), 0, sizeof(UNICODE_STRING));
			if (u16_ntpath == NULL)
			{
				errno = ENOMEM;
				return NULL;
			}

			u16_ntpath->Length = 24;
			u16_ntpath->MaximumLength = u16_ntpath->Length;
			u16_ntpath->Buffer = L"\\Device\\Null";
			*type = NULL_HANDLE;

			return u16_ntpath;
		}
		if (strncmp(path + 5, "tty", 4) == 0)
		{
			u16_ntpath = (UNICODE_STRING *)RtlAllocateHeap(NtCurrentProcessHeap(), 0, sizeof(UNICODE_STRING));
			if (u16_ntpath == NULL)
			{
				errno = ENOMEM;
				return NULL;
			}

			u16_ntpath->Length = 44;
			u16_ntpath->MaximumLength = u16_ntpath->Length;
			u16_ntpath->Buffer = L"\\Device\\ConDrv\\Console";
			*type = CONSOLE_HANDLE;

			return u16_ntpath;
		}
		if (strncmp(path + 5, "std", 3) == 0)
		{
			fdinfo info;
			int fd;
			if (strncmp(path + 8, "in", 3) == 0)
			{
				fd = 0;
				get_fdinfo(fd, &info);

				if (info.type == INVALID_HANDLE)
				{
					errno = ENOENT;
					return NULL;
				}

				u16_ntpath = get_fd_ntpath(fd);
				if (u16_ntpath == NULL)
				{
					errno = ENOMEM;
					return NULL;
				}

				*type = info.type;

				return u16_ntpath;
			}
			if (strncmp(path + 8, "out", 4) == 0)
			{
				fd = 1;
				get_fdinfo(fd, &info);

				if (info.type == INVALID_HANDLE)
				{
					errno = ENOENT;
					return NULL;
				}

				u16_ntpath = get_fd_ntpath(fd);
				if (u16_ntpath == NULL)
				{
					errno = ENOMEM;
					return NULL;
				}

				*type = info.type;

				return u16_ntpath;
			}
			if (strncmp(path + 8, "err", 4) == 0)
			{
				fd = 2;
				get_fdinfo(fd, &info);

				if (info.type == INVALID_HANDLE)
				{
					errno = ENOENT;
					return NULL;
				}

				u16_ntpath = get_fd_ntpath(fd);
				if (u16_ntpath == NULL)
				{
					errno = ENOMEM;
					return NULL;
				}

				*type = info.type;

				return u16_ntpath;
			}
		}

		// Other devices to be implemented via drivers. TODO
		if (strncmp(path + 5, "zero", 5) == 0)
		{
			u16_ntpath = (UNICODE_STRING *)RtlAllocateHeap(NtCurrentProcessHeap(), 0, sizeof(UNICODE_STRING));
			if (u16_ntpath == NULL)
			{
				errno = ENOMEM;
				return NULL;
			}

			u16_ntpath->Length = 24;
			u16_ntpath->MaximumLength = u16_ntpath->Length;
			u16_ntpath->Buffer = L"\\Device\\Zero";
			*type = NULL_HANDLE;

			return u16_ntpath;
		}
		if (strncmp(path + 5, "full", 5) == 0)
		{
			u16_ntpath = (UNICODE_STRING *)RtlAllocateHeap(NtCurrentProcessHeap(), 0, sizeof(UNICODE_STRING));
			if (u16_ntpath == NULL)
			{
				errno = ENOMEM;
				return NULL;
			}

			u16_ntpath->Length = 24;
			u16_ntpath->MaximumLength = u16_ntpath->Length;
			u16_ntpath->Buffer = L"\\Device\\Full";
			*type = NULL_HANDLE;

			return u16_ntpath;
		}
		if (strncmp(path + 5, "random", 7) == 0)
		{
			u16_ntpath = (UNICODE_STRING *)RtlAllocateHeap(NtCurrentProcessHeap(), 0, sizeof(UNICODE_STRING));
			if (u16_ntpath == NULL)
			{
				errno = ENOMEM;
				return NULL;
			}

			u16_ntpath->Length = 28;
			u16_ntpath->MaximumLength = u16_ntpath->Length;
			u16_ntpath->Buffer = L"\\Device\\Random";
			*type = NULL_HANDLE;

			return u16_ntpath;
		}
		if (strncmp(path + 5, "urandom", 8) == 0)
		{
			u16_ntpath = (UNICODE_STRING *)RtlAllocateHeap(NtCurrentProcessHeap(), 0, sizeof(UNICODE_STRING));
			if (u16_ntpath == NULL)
			{
				errno = ENOMEM;
				return NULL;
			}

			u16_ntpath->Length = 28;
			u16_ntpath->MaximumLength = u16_ntpath->Length;
			u16_ntpath->Buffer = L"\\Device\\Random";
			*type = NULL_HANDLE;

			return u16_ntpath;
		}
		if (strncmp(path + 5, "fd/", 3) == 0)
		{
			fdinfo info;
			int fd = atoi(path + 8);

			if (fd < 0)
			{
				errno = ENOENT;
				return NULL;
			}

			get_fdinfo(fd, &info);

			if (info.type == INVALID_HANDLE)
			{
				errno = ENOENT;
				return NULL;
			}

			u16_ntpath = get_fd_ntpath(fd);
			if (u16_ntpath == NULL)
			{
				errno = ENOMEM;
				return NULL;
			}

			*type = info.type;

			return u16_ntpath;
		}
	}

	// DOS devices
	if (stricmp(path, "NUL") == 0)
	{
		u16_ntpath = (UNICODE_STRING *)RtlAllocateHeap(NtCurrentProcessHeap(), 0, sizeof(UNICODE_STRING));
		if (u16_ntpath == NULL)
		{
			errno = ENOMEM;
			return NULL;
		}

		u16_ntpath->Length = 24;
		u16_ntpath->MaximumLength = u16_ntpath->Length;
		u16_ntpath->Buffer = L"\\Device\\Null";
		*type = NULL_HANDLE;

		return u16_ntpath;
	}
	if (strnicmp(path, "CON", 3) == 0)
	{
		if (path[3] == '\0')
		{
			u16_ntpath = (UNICODE_STRING *)RtlAllocateHeap(NtCurrentProcessHeap(), 0, sizeof(UNICODE_STRING));
			if (u16_ntpath == NULL)
			{
				errno = ENOMEM;
				return NULL;
			}

			u16_ntpath->Length = 44;
			u16_ntpath->MaximumLength = u16_ntpath->Length;
			u16_ntpath->Buffer = L"\\Device\\ConDrv\\Console";
			*type = CONSOLE_HANDLE;

			return u16_ntpath;
		}
		if (strnicmp(path + 3, "IN$", 4) == 0)
		{
			u16_ntpath = (UNICODE_STRING *)RtlAllocateHeap(NtCurrentProcessHeap(), 0, sizeof(UNICODE_STRING));
			if (u16_ntpath == NULL)
			{
				errno = ENOMEM;
				return NULL;
			}

			u16_ntpath->Length = 48;
			u16_ntpath->MaximumLength = u16_ntpath->Length;
			u16_ntpath->Buffer = L"\\Device\\ConDrv\\CurrentIn";
			*type = CONSOLE_HANDLE;

			return u16_ntpath;
		}
		if (strnicmp(path + 3, "OUT$", 5) == 0)
		{
			u16_ntpath = (UNICODE_STRING *)RtlAllocateHeap(NtCurrentProcessHeap(), 0, sizeof(UNICODE_STRING));
			if (u16_ntpath == NULL)
			{
				errno = ENOMEM;
				return NULL;
			}

			u16_ntpath->Length = 50;
			u16_ntpath->MaximumLength = u16_ntpath->Length;
			u16_ntpath->Buffer = L"\\Device\\ConDrv\\CurrentOut";
			*type = CONSOLE_HANDLE;

			return u16_ntpath;
		}
	}

	// Pipes
	if (strnicmp(path, "\\\\.\\pipe\\", 9) == 0)
	{
		USHORT length = (USHORT)strlen(path) - 9; // Length of the pipe name only.

		required_size = 18 * sizeof(WCHAR);            // '\Device\NamedPipe\'.
		required_size += (length * sizeof(WCHAR)) + 2; // L'\0'.

		u16_ntpath = (UNICODE_STRING *)RtlAllocateHeap(NtCurrentProcessHeap(), 0, sizeof(UNICODE_STRING) + required_size);
		if (u16_ntpath == NULL)
		{
			errno = ENOMEM;
			return NULL;
		}

		u16_ntpath->Buffer = (WCHAR *)((char *)u16_ntpath + sizeof(UNICODE_STRING));
		u16_ntpath->Length = 0;
		u16_ntpath->MaximumLength = required_size;

		u8_path.Buffer = (char *)path + 9;
		u8_path.Length = length;
		u8_path.MaximumLength = length;

		status = RtlUTF8StringToUnicodeString(u16_ntpath, &u8_path, FALSE);

		if (status != STATUS_SUCCESS)
		{
			RtlFreeHeap(NtCurrentProcessHeap(), 0, u16_ntpath);
			return NULL;
		}

		memmove((char *)u16_ntpath->Buffer + 18 * sizeof(WCHAR), u16_ntpath->Buffer, u16_ntpath->Length);
		memcpy(u16_ntpath->Buffer, L"\\Device\\NamedPipe\\", 18 * sizeof(WCHAR));

		u16_ntpath->Length += 18 * sizeof(WCHAR);
		u16_ntpath->Buffer[u16_ntpath->Length / sizeof(WCHAR)] = L'\0'; // Null terminate the path.
		u16_ntpath->MaximumLength = u16_ntpath->Length + sizeof(WCHAR);

		*type = PIPE_HANDLE;
		return u16_ntpath;
	}

	// After this point the path will be either a FILE_HANDLE or DIRECTORY_HANDLE. Set it to FILE_HANDLE.
	*type = FILE_HANDLE;

	// Network Shares
	if ((path[0] == '\\' && path[1] == '\\') || (path[0] == '/' && path[1] == '/'))
	{
		RtlInitUTF8String(&u8_path, path);
		status = RtlUTF8StringToUnicodeString(&u16_path, &u8_path, TRUE);

		if (status != STATUS_SUCCESS)
		{
			map_ntstatus_to_errno(status);
			return NULL;
		}

		// Convert forward slashes to backward slashes
		for (int i = 0; u16_path.Buffer[i] != L'\0'; ++i)
		{
			if (u16_path.Buffer[i] == L'/')
			{
				u16_path.Buffer[i] = L'\\';
			}
		}

		required_size = (u8_path.Length + 12 - 1) * sizeof(WCHAR); // (+) "\Device\Mup\" (-) "\\" (+) '\0'
		ntpath_buffer = (WCHAR *)RtlAllocateHeap(NtCurrentProcessHeap(), 0, required_size);

		if (ntpath_buffer == NULL)
		{
			RtlFreeUnicodeString(&u16_path);
			errno = ENOMEM;
			return NULL;
		}

		memcpy(ntpath_buffer, L"\\Device\\Mup\\", 12 * sizeof(WCHAR));
		memcpy((CHAR *)ntpath_buffer + 12 * sizeof(WCHAR), &(u16_path.Buffer[2]), u16_path.Length - sizeof(WCHAR));

		RtlFreeUnicodeString(&u16_path);

		goto path_coalesce;
	}

	// Temporary directory.
	if (strnicmp(path, "/tmp", 4) == 0)
	{
		char *new_path;
		char *tmp = wlibc_tmpdir();

		if (tmp == NULL)
		{
			errno = ENOENT;
			goto finish;
		}

		size_t path_length = strlen(path);
		size_t tmp_length = strlen(tmp);

		required_size = (USHORT)(tmp_length + path_length - 4 + 1);
		new_path = (char *)RtlAllocateHeap(NtCurrentProcessHeap(), 0, required_size);

		if (new_path == NULL)
		{
			errno = ENOMEM;
			return NULL;
		}

		memcpy(new_path, tmp, tmp_length);
		memcpy(new_path + tmp_length, path + 4, path_length - 3);

		path = new_path;
		temp_path_requested = true;
	}

	if (IS_ABSOLUTE_PATH(path))
	{
		path_is_absolute = true;
		if (path[0] == '/')
		{
			if (isalpha(path[1]) && (path[2] == '/' || path[2] == '\0'))
			{
				cygwin_path = true;
			}
			else // /abcd (Bad path).
			{
				errno = ENOENT;
				return NULL;
			}
		}
	}
	else
	{
		if (dirfd == AT_FDCWD)
		{
			/*
			   Calling `NtQueryObject` on the directory handle every time is slow.
			   There is no way of knowing whether the current working directory has changed
			   based on the handle and buffer as they are reused for subsequent `SetCurrentDirectory` calls.
			   We rely on the dospath and change the drive letter to it's NT device name. Since we cache the
			   devices this approach is faster and no overhead of syscalls happen here (except for the very first call ofcourse).
			*/
			PUNICODE_STRING pu16_cwd;
			// TODO Locking
			pu16_cwd = &NtCurrentPeb()->ProcessParameters->CurrentDirectory.DosPath;
			// Eg "C:\Windows\"" -> "\Windows\"
			short cwd_length_without_drive_letter = pu16_cwd->Length - 2 * sizeof(WCHAR);
			rootdir_buffer = (char *)RtlAllocateHeap(NtCurrentProcessHeap(), 0, 1024);

			if (rootdir_buffer == NULL)
			{
				errno = ENOMEM;
				goto finish;
			}

			// No need to check return value here as RTL_USER_PROCESS_PARAMETERS can be trusted.
			// UTF-16LE is just zero extended ASCII for the english alphabet.
			// char truncation will get the volume label.
			device = dos_device_to_nt_device((char)pu16_cwd->Buffer[0]);

			memcpy(rootdir_buffer, device->name, device->length);
			memcpy(rootdir_buffer + device->length, (char *)pu16_cwd->Buffer + (2 * sizeof(WCHAR)), cwd_length_without_drive_letter);

			u16_rootdir.Length = device->length + cwd_length_without_drive_letter;
			u16_rootdir.MaximumLength = u16_rootdir.Length;
			u16_rootdir.Buffer = (WCHAR *)rootdir_buffer;

			// DosPath always has a trailing slash.
			rootdir_has_trailing_slash = true;
		}
		else
		{
			rootdir_buffer = (char *)get_fd_ntpath_internal(dirfd);
			if (rootdir_buffer == NULL)
			{
				// Bad file descriptor for directory.
				// This really should not happen as dirfd is validated before this function call, but just in case.
				errno = EBADF;
				goto finish;
			}

			u16_rootdir.Length = ((PUNICODE_STRING)rootdir_buffer)->Length;
			u16_rootdir.MaximumLength = ((PUNICODE_STRING)rootdir_buffer)->MaximumLength;
			u16_rootdir.Buffer = ((PUNICODE_STRING)rootdir_buffer)->Buffer;

			// Check to see if the path has a trailing slash. Most likely it will not.
			if (u16_rootdir.Buffer[u16_rootdir.Length / sizeof(WCHAR) - 1] == L'\\')
			{
				rootdir_has_trailing_slash = true;
			}

			// This is to prevent freeing the pointer.
			rootdir_buffer = NULL;
		}
	}

	RtlInitUTF8String(&u8_path, path);
	status = RtlUTF8StringToUnicodeString(&u16_path, &u8_path, TRUE);

	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		goto finish;
	}

	// Convert forward slashes to backward slashes
	for (int i = 0; u16_path.Buffer[i] != L'\0'; ++i)
	{
		if (u16_path.Buffer[i] == L'/')
		{
			u16_path.Buffer[i] = L'\\';
		}
	}

	required_size = u16_path.MaximumLength; // Includes L'\0'

	if (!path_is_absolute)
	{
		required_size += u16_rootdir.Length + 2; // L'\'
	}
	else // absolute path
	{
		required_size += 54; // "\\Device\\HarddiskVolumeXXXX\\"
	}

	ntpath_buffer = (WCHAR *)RtlAllocateHeap(NtCurrentProcessHeap(), 0, required_size);
	if (ntpath_buffer == NULL)
	{
		errno = ENOMEM;
		goto finish;
	}

	if (!path_is_absolute)
	{
		memcpy((char *)ntpath_buffer, u16_rootdir.Buffer, u16_rootdir.Length);
		if (!rootdir_has_trailing_slash)
		{
			ntpath_buffer[u16_rootdir.Length / sizeof(WCHAR)] = L'\\';
			u16_rootdir.Length += 2;
		}
		memcpy((char *)ntpath_buffer + u16_rootdir.Length, u16_path.Buffer, u16_path.MaximumLength);
	}
	else // absolute path
	{
		char volume;

		// Get the NT device first
		if (!cygwin_path)
		{
			volume = (char)toupper(path[0]);
		}
		else
		{
			volume = (char)toupper(path[1]);
		}

		device = dos_device_to_nt_device(volume);
		if (device == NULL)
		{
			// Bad device
			errno = ENOENT;
			goto finish;
		}

		memcpy((char *)ntpath_buffer, device->name, device->length);
		memcpy((char *)ntpath_buffer + device->length, (char *)u16_path.Buffer + (2 * sizeof(WCHAR)),
			   u16_path.MaximumLength - (2 * sizeof(WCHAR)));
	}

	RtlFreeUnicodeString(&u16_path);

path_coalesce:
	/*
	   This works like a stack.
	   When the component is other than '..' or '.' we push the contents onto the stack.
	   When the component is '..' the stack is popped.
	   This is a zero copy stack, i.e the components are not copied at all.
	   We use the starting index and the length(in bytes) to reference each component.
	*/
	int index = 0;
	int start = 0;
	for (int i = 8;; i++) // start after \Device\:
	{
		if (ntpath_buffer[i] == L'\\' || ntpath_buffer[i] == L'\0')
		{
			if (i - start > 2) // not '.' or '..'
			{
				void *temp = add_component(components, &index, start, (i - start) * sizeof(WCHAR)); // push stack
				if (temp == NULL)
				{
					goto finish;
				}

				components = temp;
			}
			else
			{
				if (i - start == 2 && memcmp(ntpath_buffer + start, L"..", 4) == 0)
				{
					--index; // pop stack
					if (index < 1)
					{
						// root path -> C:/.. -> C:/, C:/../.. -> C:/
						index = 1;
					}
				}
				else if (i - start == 1 && memcmp(ntpath_buffer + start, L".\\", 2) == 0)
				{
					; // do nothing
				}
				else
				{
					void *temp = add_component(components, &index, start, (i - start) * sizeof(WCHAR)); // push stack
					if (temp == NULL)
					{
						goto finish;
					}

					components = temp;
				}
			}

			if (ntpath_buffer[i] == L'\0')
			{
				break;
			}
			start = i + 1;
		}
	}

	// All the constructed paths will have a terminating NULL.
	u16_ntpath = (UNICODE_STRING *)RtlAllocateHeap(NtCurrentProcessHeap(), 0, required_size + sizeof(UNICODE_STRING));
	if (u16_ntpath == NULL)
	{
		errno = ENOMEM;
		goto finish;
	}

	u16_ntpath->Length = 0;
	u16_ntpath->MaximumLength = required_size;
	u16_ntpath->Buffer = (WCHAR *)((char *)u16_ntpath + sizeof(UNICODE_STRING));

	for (int i = 0; i < index; i++)
	{
		memcpy((char *)u16_ntpath->Buffer + u16_ntpath->Length, ntpath_buffer + components[i].start, components[i].length);
		u16_ntpath->Length += (USHORT)components[i].length;
		if (i + 1 < index)
		{
			u16_ntpath->Buffer[u16_ntpath->Length / sizeof(WCHAR)] = L'\\';
			u16_ntpath->Length += 2;
		}
	}

	if (index == 1)
	{
		// The case where we resolve a volume. eg C:
		// Always add trailing slash to the volume so that it can be treated as a directory by the NT calls.
		u16_ntpath->Buffer[u16_ntpath->Length / sizeof(WCHAR)] = L'\\';
		u16_ntpath->Length += sizeof(WCHAR);
	}

	u16_ntpath->Buffer[u16_ntpath->Length / sizeof(WCHAR)] = L'\0';

finish:
	RtlFreeHeap(NtCurrentProcessHeap(), 0, components);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, rootdir_buffer);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, ntpath_buffer);

	if (temp_path_requested)
	{
		RtlFreeHeap(NtCurrentProcessHeap(), 0, (void *)path);
	}

	return u16_ntpath;
}

UNICODE_STRING *get_fd_ntpath(int fd)
{
	UNICODE_STRING *ntpath = NULL;
	UNICODE_STRING *ntpath_copy = NULL;

	ntpath = get_fd_ntpath_internal(fd);
	ntpath_copy = (UNICODE_STRING *)RtlAllocateHeap(NtCurrentProcessHeap(), 0, sizeof(UNICODE_STRING) + ntpath->MaximumLength);

	if (ntpath_copy == NULL)
	{
		errno = ENOMEM;
		return NULL;
	}

	memcpy(ntpath_copy, ntpath, sizeof(UNICODE_STRING) + ntpath->MaximumLength);

	return ntpath_copy;
}

/*
   To convert NT device names to dos devices, we use a cache with 2 slots.
   The first cache slot is the one that is expected to be hit most of the time.
   The second one is expected to be hit rarely since most applications will not
   change their working directory to different drive frequently. Once an application
   changes to a different drive and calls any routine that uses this function, the
   second cache slot will be promoted to the first slot and the the first slot demoted
   to second.
*/
dos_device dos_device_cache[2] = {{(dev_t)-1, 0}, {(dev_t)-1, 0}};

UNICODE_STRING *ntpath_to_dospath(const UNICODE_STRING *ntpath)
{
	UNICODE_STRING *dospath = NULL;
	UNICODE_STRING nt_device_path;
	dev_t device_number = 0;
	char label = '\0';
	int second_slash_pos = 0;
	bool second_slash_found = false;

	// Special devices
	if (memcmp(ntpath->Buffer, L"\\Device\\Null", 13 * sizeof(WCHAR)) == 0)
	{
		dospath = (UNICODE_STRING *)RtlAllocateHeap(NtCurrentProcessHeap(), 0, sizeof(UNICODE_STRING));
		if (dospath == NULL)
		{
			errno = ENOMEM;
			return NULL;
		}

		dospath->Buffer = L"NUL";
		dospath->Length = 3 * sizeof(WCHAR);
		dospath->MaximumLength = dospath->Length;

		return dospath;
	}
	if (memcmp(ntpath->Buffer, L"\\Device\\ConDrv", 14 * sizeof(WCHAR)) == 0)
	{
		dospath = (UNICODE_STRING *)RtlAllocateHeap(NtCurrentProcessHeap(), 0, sizeof(UNICODE_STRING));
		if (dospath == NULL)
		{
			errno = ENOMEM;
			return NULL;
		}

		dospath->Buffer = L"CON";
		dospath->Length = 3 * sizeof(WCHAR);
		dospath->MaximumLength = dospath->Length;

		return dospath;
	}

	// Pipes
	if (memcmp(ntpath->Buffer, L"\\Device\\NamedPipe", 17 * sizeof(WCHAR)) == 0)
	{
		dospath = (UNICODE_STRING *)RtlAllocateHeap(NtCurrentProcessHeap(), 0,
													sizeof(UNICODE_STRING) + ntpath->Length - ((17 - 9) * sizeof(WCHAR)));
		if (dospath == NULL)
		{
			errno = ENOMEM;
			return NULL;
		}

		dospath->Buffer = (WCHAR *)((CHAR *)dospath + sizeof(UNICODE_STRING));

		memcpy(dospath->Buffer, L"\\\\.\\pipe", 8 * sizeof(WCHAR));
		dospath->Length = 8 * sizeof(WCHAR);

		memcpy((CHAR *)dospath->Buffer + dospath->Length, (CHAR *)ntpath->Buffer + (17 * sizeof(WCHAR)),
			   ntpath->Length - (16 * sizeof(WCHAR)));
		dospath->Length += ntpath->Length - (17 * sizeof(WCHAR)); // Exclude L'\0'

		dospath->MaximumLength = dospath->Length + sizeof(WCHAR);

		return dospath;
	}

	// Network shares
	if (memcmp(ntpath->Buffer, L"\\Device\\Mup", 11 * sizeof(WCHAR)) == 0)
	{
		dospath = (UNICODE_STRING *)RtlAllocateHeap(NtCurrentProcessHeap(), 0,
													sizeof(UNICODE_STRING) + ntpath->Length - ((11 - 2) * sizeof(WCHAR)));
		if (dospath == NULL)
		{
			errno = ENOMEM;
			return NULL;
		}

		dospath->Buffer = (WCHAR *)((CHAR *)dospath + sizeof(UNICODE_STRING));

		memcpy(dospath->Buffer, L"\\", sizeof(WCHAR));
		dospath->Length = sizeof(WCHAR);

		memcpy((CHAR *)dospath->Buffer + dospath->Length, (CHAR *)ntpath->Buffer + (11 * sizeof(WCHAR)),
			   ntpath->Length - (10 * sizeof(WCHAR)));
		dospath->Length += ntpath->Length - (11 * sizeof(WCHAR)); // Exclude L'\0'

		dospath->MaximumLength = dospath->Length + sizeof(WCHAR);

		return dospath;
	}

	// Perform a simple 'atoi' (UTF16-LE).
	// Find the second slash as well if present.
	for (int i = 22; ntpath->Buffer[i] != L'\0'; ++i) // skip "\Device\HarddiskVolume"
	{
		if (ntpath->Buffer[i] == L'\\')
		{
			second_slash_pos = i;
			second_slash_found = true;
			break;
		}

		device_number = device_number * 10 + (dev_t)(ntpath->Buffer[i] - L'0');
	}

	// Check the cache first.
	// slot 1
	if (dos_device_cache[0].device_number == device_number)
	{
		label = dos_device_cache[0].label;
	}
	// slot 2
	else if (dos_device_cache[1].device_number == device_number)
	{
		label = dos_device_cache[1].label;
		// If the second slot is hit, swap it with the first slot.
		dos_device temp = dos_device_cache[0];
		dos_device_cache[0] = dos_device_cache[1];
		dos_device_cache[1] = temp;
	}
	// Not in cache
	else
	{
		// Only consider "\Device\HarddiskVolumeXXXX" without the next slash.
		nt_device_path.Buffer = ntpath->Buffer;
		nt_device_path.Length = second_slash_found ? (USHORT)(second_slash_pos * sizeof(WCHAR)) : ntpath->Length;
		nt_device_path.MaximumLength = nt_device_path.Length;

		label = nt_device_to_dos_device(&nt_device_path);

		if (label != '\0')
		{
			// Put the entry in the cache.
			// Check if the first slot is populated if not, always put the new entry
			// in the second slot.
			if (dos_device_cache[0].device_number == (dev_t)-1)
			{
				dos_device_cache[0].device_number = device_number;
				dos_device_cache[0].label = label;
			}
			else
			{
				dos_device_cache[1].device_number = device_number;
				dos_device_cache[1].label = label;
			}
		}
	}

	if (label != '\0')
	{
		// We technically will actually use less than this, but this is close enough. ((-) "\Device\HarddiskVolume" (+) "C:").
		dospath =
			(UNICODE_STRING *)RtlAllocateHeap(NtCurrentProcessHeap(), 0, sizeof(UNICODE_STRING) + ntpath->Length - (16 * sizeof(WCHAR)));
		if (dospath == NULL)
		{
			errno = ENOMEM;
			return NULL;
		}

		dospath->Buffer = (WCHAR *)((char *)dospath + sizeof(UNICODE_STRING));
		dospath->Buffer[0] = (WCHAR)label;
		dospath->Buffer[1] = L':';

		if (second_slash_found)
		{
			memcpy(dospath->Buffer + 2, ntpath->Buffer + second_slash_pos, ntpath->Length - (second_slash_pos * sizeof(WCHAR)));
			dospath->Length = (USHORT)(ntpath->Length - ((second_slash_pos - 2) * sizeof(WCHAR)));
			dospath->MaximumLength = dospath->Length + sizeof(WCHAR);
			dospath->Buffer[dospath->Length / sizeof(WCHAR)] = L'\0';
		}
		else // just the drive, eg. C:
		{
			dospath->Buffer[2] = L'\0';
			dospath->Length = 2 * sizeof(WCHAR);
			dospath->MaximumLength = 3 * sizeof(WCHAR);
		}
	}

	return dospath;
}

UNICODE_STRING *dospath_to_ntpath(const UNICODE_STRING *dospath)
{
	nt_device *device = NULL;
	UNICODE_STRING *ntpath = NULL;

	device = dos_device_to_nt_device((char)dospath->Buffer[0]);
	if (device == NULL)
	{
		return NULL;
	}

	ntpath =
		(UNICODE_STRING *)RtlAllocateHeap(NtCurrentProcessHeap(), 0, sizeof(UNICODE_STRING) + device->length + dospath->MaximumLength - 4);
	if (ntpath == NULL)
	{
		errno = ENOMEM;
		return NULL;
	}

	memcpy((CHAR *)ntpath + sizeof(UNICODE_STRING), device->name, device->length);
	memcpy((CHAR *)ntpath + sizeof(UNICODE_STRING) + device->length, (CHAR *)dospath->Buffer + 4, dospath->MaximumLength - 4);

	ntpath->Buffer = (WCHAR *)((CHAR *)ntpath + sizeof(UNICODE_STRING));
	ntpath->Length = device->length + dospath->Length - 4;
	ntpath->MaximumLength = ntpath->Length + sizeof(WCHAR);

	return ntpath;
}

UNICODE_STRING *get_absolute_dospath(int dirfd, const char *path)
{
	UNICODE_STRING *ntpath = NULL;
	UNICODE_STRING *dospath = NULL;

	// First get the NT path, then convert it to a DOS path.
	ntpath = get_absolute_ntpath(dirfd, path);
	if (ntpath == NULL)
	{
		// errno wil be set by 'get_absolute_ntpath'.
		return NULL;
	}

	dospath = ntpath_to_dospath(ntpath);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, ntpath);

	return dospath;
}

UNICODE_STRING *get_fd_dospath(int fd)
{
	UNICODE_STRING *ntpath = get_fd_ntpath_internal(fd);
	if (ntpath == NULL)
	{
		return NULL;
	}

	return ntpath_to_dospath(ntpath);
}
