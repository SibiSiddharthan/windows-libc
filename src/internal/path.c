/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/fcntl.h>
#include <internal/path.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>

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
		components = (path_component *)malloc(path_components_size * sizeof(path_component));
	}

	if (*index == path_components_size)
	{
		void *temp = malloc(path_components_size * 2 * sizeof(path_component));
		memcpy(temp, components, path_components_size * sizeof(path_component));
		free(components);
		components = temp;
		path_components_size *= 2;
	}

	components[*index].start = start;
	components[*index].length = length;
	(*index)++;
	return components;
}

typedef struct
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
				devices[volume - 'A'].length = -1;
				return NULL;
			}

			// Use the cache structures buffer itself
			realpath.Buffer = devices[volume - 'A'].name;
			realpath.Length = 0;
			realpath.MaximumLength = 26 * sizeof(WCHAR);

			status = NtQuerySymbolicLinkObject(handle, &realpath, NULL);
			if (status != STATUS_SUCCESS)
			{
				// Mark the volume as non existent.
				devices[volume - 'A'].length = -1;
				return NULL;
			}

			NtClose(handle);

			devices[volume - 'A'].length = realpath.Length;
			return &devices[volume - 'A'];
		}
	}

	return NULL;
}

// void nt_device_to_dos_device();

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
		free(fd_path_cache[index].nt_path);
	}
	fd_path_cache[index].nt_path = nt_path;
}

// NOTE: The returned pointer should not be freed.
UNICODE_STRING *get_fd_ntpath_internal(int fd)
{
	NTSTATUS status;
	HANDLE handle;
	ULONG length;
	UNICODE_STRING *path = NULL;
	int sequence;

	SHARED_LOCK_FD_TABLE();
	sequence = _wlibc_fd_table[fd].sequence;
	handle = _wlibc_fd_table[fd].handle;
	SHARED_UNLOCK_FD_TABLE();

	// Check the cache first.
	path = check_fd_path_cache(fd, sequence);
	if (path != NULL)
	{
		return path;
	}

	// Not in cache do the lookup.
	path = malloc(1024);
	status = NtQueryObject(handle, ObjectNameInformation, path, 1024, &length);

	if (status == STATUS_BUFFER_OVERFLOW)
	{
		path = realloc(path, length);
		status = NtQueryObject(handle, ObjectNameInformation, path, length, &length);
	}

	if (status != STATUS_SUCCESS)
	{
		return NULL;
	}

	// Update the cache.
	update_fd_path_cache(fd, sequence, path);

	return path;
}

UNICODE_STRING *get_absolute_ntpath2(int dirfd, const char *path, handle_t *type)
{
	UTF8_STRING u8_path;
	UNICODE_STRING u16_path, u16_rootdir = {0};

	// User should free the allocated memory.
	UNICODE_STRING *u16_ntpath = NULL;

	nt_device *device = NULL;
	// malloc'ed resources
	path_component *components = NULL;
	char *rootdir_buffer = NULL;
	WCHAR *ntpath_buffer = NULL;

	bool path_is_absolute = false;
	bool cygwin_path = false;
	bool rootdir_has_trailing_slash = false;

	handle_t unused;

	if (type == NULL)
	{
		type = &unused;
	}

	// For all these predefined devices we set MaximumLength to be same as Length (omitting the terminating NULL).
	// This enables us to use the static storage an avoids needless memory allocation.

	// ROOT
	if (path[1] == '\0' && (path[0] == '/' || path[0] == '\\'))
	{
		// We should be able to stat root as many programs need this.
		// Here the root will refer to the current drive of the process.

		// UTF-16LE char truncation.
		device = dos_device_to_nt_device((char)(NtCurrentPeb()->ProcessParameters->CurrentDirectory.DosPath.Buffer[0]));
		u16_ntpath = (UNICODE_STRING *)malloc(sizeof(UNICODE_STRING) + device->length + 2 * sizeof(WCHAR)); // '\\', '\0'

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
			u16_ntpath = (UNICODE_STRING *)malloc(sizeof(UNICODE_STRING));
			u16_ntpath->Length = 24;
			u16_ntpath->MaximumLength = u16_ntpath->Length;
			u16_ntpath->Buffer = L"\\Device\\Null";
			*type = NULL_HANDLE;
			return u16_ntpath;
		}
		if (strncmp(path + 5, "tty", 4) == 0)
		{
			u16_ntpath = (UNICODE_STRING *)malloc(sizeof(UNICODE_STRING));
			u16_ntpath->Length = 44;
			u16_ntpath->MaximumLength = u16_ntpath->Length;
			u16_ntpath->Buffer = L"\\Device\\ConDrv\\Console";
			*type = CONSOLE_HANDLE;
			return u16_ntpath;
		}
		if (strncmp(path + 5, "stdin", 6) == 0)
		{
			u16_ntpath = (UNICODE_STRING *)malloc(sizeof(UNICODE_STRING));
			u16_ntpath->Length = 48;
			u16_ntpath->MaximumLength = u16_ntpath->Length;
			u16_ntpath->Buffer = L"\\Device\\ConDrv\\CurrentIn";
			*type = CONSOLE_HANDLE;
			return u16_ntpath;
		}
		if (strncmp(path + 5, "stdout", 7) == 0)
		{
			u16_ntpath = (UNICODE_STRING *)malloc(sizeof(UNICODE_STRING));
			u16_ntpath->Length = 50;
			u16_ntpath->MaximumLength = u16_ntpath->Length;
			u16_ntpath->Buffer = L"\\Device\\ConDrv\\CurrentOut";
			*type = CONSOLE_HANDLE;
			return u16_ntpath;
		}
		if (strncmp(path + 5, "stderr", 7) == 0)
		{
			u16_ntpath = (UNICODE_STRING *)malloc(sizeof(UNICODE_STRING));
			u16_ntpath->Length = 50;
			u16_ntpath->MaximumLength = u16_ntpath->Length;
			u16_ntpath->Buffer = L"\\Device\\ConDrv\\CurrentOut";
			*type = CONSOLE_HANDLE;
			return u16_ntpath;
		}
		// Other devices to be implemented via drivers. TODO
		if (strncmp(path + 5, "zero", 5) == 0)
		{
			u16_ntpath = (UNICODE_STRING *)malloc(sizeof(UNICODE_STRING));
			u16_ntpath->Length = 24;
			u16_ntpath->MaximumLength = u16_ntpath->Length;
			u16_ntpath->Buffer = L"\\Device\\Zero";
			*type = NULL_HANDLE;
			return u16_ntpath;
		}
		if (strncmp(path + 5, "full", 5) == 0)
		{
			u16_ntpath->Length = 24;
			u16_ntpath->MaximumLength = u16_ntpath->Length;
			u16_ntpath->Buffer = L"\\Device\\Full";
			*type = NULL_HANDLE;
			return u16_ntpath;
		}
		if (strncmp(path + 5, "random", 7) == 0)
		{
			u16_ntpath = (UNICODE_STRING *)malloc(sizeof(UNICODE_STRING));
			u16_ntpath->Length = 28;
			u16_ntpath->MaximumLength = u16_ntpath->Length;
			u16_ntpath->Buffer = L"\\Device\\Random";
			*type = NULL_HANDLE;
			return u16_ntpath;
		}
		if (strncmp(path + 5, "urandom", 8) == 0)
		{
			u16_ntpath = (UNICODE_STRING *)malloc(sizeof(UNICODE_STRING));
			u16_ntpath->Length = 28;
			u16_ntpath->MaximumLength = u16_ntpath->Length;
			u16_ntpath->Buffer = L"\\Device\\Random";
			*type = NULL_HANDLE;
			return u16_ntpath;
		}
	}

	// DOS devices
	if (stricmp(path, "NUL") == 0)
	{
		u16_ntpath = (UNICODE_STRING *)malloc(sizeof(UNICODE_STRING));
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
			u16_ntpath = (UNICODE_STRING *)malloc(sizeof(UNICODE_STRING));
			u16_ntpath->Length = 44;
			u16_ntpath->MaximumLength = u16_ntpath->Length;
			u16_ntpath->Buffer = L"\\Device\\ConDrv\\Console";
			*type = CONSOLE_HANDLE;
			return u16_ntpath;
		}
		if (strnicmp(path + 3, "IN$", 4) == 0)
		{
			u16_ntpath = (UNICODE_STRING *)malloc(sizeof(UNICODE_STRING));
			u16_ntpath->Length = 48;
			u16_ntpath->MaximumLength = u16_ntpath->Length;
			u16_ntpath->Buffer = L"\\Device\\ConDrv\\CurrentIn";
			*type = CONSOLE_HANDLE;
			return u16_ntpath;
		}
		if (strnicmp(path + 3, "OUT$", 5) == 0)
		{
			u16_ntpath = (UNICODE_STRING *)malloc(sizeof(UNICODE_STRING));
			u16_ntpath->Length = 50;
			u16_ntpath->MaximumLength = u16_ntpath->Length;
			u16_ntpath->Buffer = L"\\Device\\ConDrv\\CurrentOut";
			*type = CONSOLE_HANDLE;
			return u16_ntpath;
		}
	}

	// After this point the path will be either a FILE_HANDLE or DIRECTORY_HANDLE. Set it to FILE_HANDLE.
	*type = FILE_HANDLE;

	if ( // Normal Windows way -> C:
		(isalpha(path[0]) && path[1] == ':') ||
		// Cygwin way /c
		path[0] == '/')
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
			rootdir_buffer = (char *)malloc(1024);

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
	RtlUTF8StringToUnicodeString(&u16_path, &u8_path, TRUE);

	// Convert forward slashes to backward slashes
	for (int i = 0; u16_path.Buffer[i] != L'\0'; ++i)
	{
		if (u16_path.Buffer[i] == L'/')
		{
			u16_path.Buffer[i] = L'\\';
		}
	}

	USHORT required_size = u16_path.MaximumLength; // Includes L'\0'

	if (!path_is_absolute)
	{
		required_size += u16_rootdir.Length + 2; // L'\'
	}
	else // absolute path
	{
		required_size += 54; // "\\Device\\HarddiskVolumeXXXX\\"
	}

	ntpath_buffer = (WCHAR *)malloc(required_size);

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
			volume = toupper(path[0]);
		}
		else
		{
			volume = toupper(path[1]);
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
				components = add_component(components, &index, start, (i - start) * sizeof(WCHAR)); // push stack
			}
			else
			{
				if (i - start == 2 && memcmp(ntpath_buffer + start, L"..", 4) == 0)
				{
					--index;       // pop stack
					if (index < 1) // bad path
					{
						errno = ENOENT;
						goto finish;
					}
				}
				else if (i - start == 1 && memcmp(ntpath_buffer + start, L".\\", 2) == 0)
				{
					; // do nothing
				}
				else
				{
					components = add_component(components, &index, start, (i - start) * sizeof(WCHAR)); // push stack
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
	u16_ntpath = (UNICODE_STRING *)malloc(required_size + sizeof(UNICODE_STRING));
	u16_ntpath->Length = 0;
	u16_ntpath->MaximumLength = required_size;
	u16_ntpath->Buffer = (WCHAR *)((char *)u16_ntpath + sizeof(UNICODE_STRING));

	for (int i = 0; i < index; i++)
	{
		memcpy((char *)u16_ntpath->Buffer + u16_ntpath->Length, ntpath_buffer + components[i].start, components[i].length);
		u16_ntpath->Length += components[i].length;
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
	free(components);
	free(rootdir_buffer);
	free(ntpath_buffer);

	return u16_ntpath;
}

UNICODE_STRING *get_fd_ntpath(int fd)
{
	UNICODE_STRING *ntpath = NULL;
	UNICODE_STRING *ntpath_copy = NULL;

	ntpath = get_fd_ntpath_internal(fd);
	ntpath_copy = (UNICODE_STRING *)malloc(sizeof(UNICODE_STRING) + ntpath->MaximumLength);
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
typedef struct _dos_device
{
	dev_t device_number;
	char label;
} dos_device;

dos_device dos_device_cache[2] = {{-1, 0}, {-1, 0}};

UNICODE_STRING *ntpath_to_dospath(const UNICODE_STRING *ntpath)
{
	UNICODE_STRING *dospath = NULL;
	nt_device *device = NULL;
	dev_t device_number = 0;
	char label = 0;

	// Perform a simple 'atoi' (UTF16-LE).
	for (int i = 22; ntpath->Buffer[i] != L'\\' && ntpath->Buffer[i] != L'\0'; ++i) // skip "\Device\HarddiskVolume"
	{
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
		for (int i = 0; i < 26; ++i)
		{
			// Iterate from A-Z.
			device = dos_device_to_nt_device(i + 'A');
			if (device != NULL)
			{
				// skip "\Device\HarddiskVolume"
				if (memcmp(device->name + 22, ntpath->Buffer + 22, device->length - (22 * sizeof(WCHAR))) == 0)
				{
					label = i + 'A';
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

					break;
				}
			}
		}
	}

	if (label != 0)
	{
		// We technically will actually use less than this, but this is close enough. ((-) "\Device\HarddiskVolume" (+) "C:").
		dospath = (UNICODE_STRING *)malloc(sizeof(UNICODE_STRING) + ntpath->Length - (16 * sizeof(WCHAR)));
		dospath->Buffer = (WCHAR *)((char *)dospath + sizeof(UNICODE_STRING));
		dospath->Buffer[0] = (WCHAR)label;
		dospath->Buffer[1] = L':';

		int j;
		bool second_slash_found = false;
		// Find the second slash
		for (j = 22; ntpath->Buffer[j] != L'\0'; ++j) // skip "\Device\HarddiskVolume"
		{
			if (ntpath->Buffer[j] == L'\\')
			{
				second_slash_found = true;
				break;
			}
		}

		if (second_slash_found)
		{
			memcpy(dospath->Buffer + 2, ntpath->Buffer + j, ntpath->Length - (j * sizeof(WCHAR)));
			dospath->Length = (USHORT)(ntpath->Length - ((j - 2) * sizeof(WCHAR)));
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

	ntpath = (UNICODE_STRING *)malloc(sizeof(UNICODE_STRING) + device->length + dospath->MaximumLength - 4);
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
	free(ntpath);

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
