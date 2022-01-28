/*
   Copyright (c) 2020-2021 Sibi Siddharthan

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

static int path_components_size = 32;

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
		void *temp = malloc(path_components_size * sizeof(path_component));
		memcpy(temp, components, path_components_size * sizeof(path_component));
		components = (path_component *)realloc(components, 2 * path_components_size * sizeof(path_component));
		memcpy(components, temp, path_components_size * sizeof(path_component));
		free(temp);
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
			return NULL;
		}

		// Use the cache structures buffer itself
		realpath.Buffer = devices[volume - 'A'].name;
		realpath.Length = 0;
		realpath.MaximumLength = 26 * sizeof(WCHAR);

		status = NtQuerySymbolicLinkObject(handle, &realpath, NULL);
		if (status != STATUS_SUCCESS)
		{
			return NULL;
		}

		NtClose(handle);

		devices[volume - 'A'].length = realpath.Length;
		return &devices[volume - 'A'];
	}

	return NULL;
}

void nt_device_to_dos_device();

void *xget_fd_path(int fd)
{
	NTSTATUS status;
	HANDLE handle;
	ULONG length;

	void *path = NULL;

	handle = get_fd_handle(fd);
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

	return path;
}

UNICODE_STRING *xget_absolute_ntpath(int dirfd, const char *path)
{
	UTF8_STRING u8_path;
	UNICODE_STRING u16_path, u16_rootdir;

	// User should free this by calling `free_ntpath`.
	UNICODE_STRING *u16_ntpath = NULL;
	static UNICODE_STRING u16_ntdevices;

	nt_device *device = NULL;
	// malloc'ed resources
	path_component *components = NULL;
	char *rootdir_buffer = NULL;
	WCHAR *ntpath_buffer = NULL;

	bool path_is_absolute = false;
	bool cygwin_path = false;

	// For all these predefined devices we set MaximumLength to be same as Length (omitting the terminating NULL).
	// This enables us to use the static storage an avoids needless memory allocation.

	// POSIX devices
	if (strncmp(path, "/dev/", 5) == 0)
	{
		if (strncmp(path + 5, "null", 5) == 0)
		{
			u16_ntdevices.Length = 24;
			u16_ntdevices.MaximumLength = u16_ntdevices.Length;
			u16_ntdevices.Buffer = L"\\Device\\Null";
			return &u16_ntdevices;
		}
		if (strncmp(path + 5, "tty", 4) == 0)
		{
			u16_ntdevices.Length = 44;
			u16_ntdevices.MaximumLength = u16_ntdevices.Length;
			u16_ntdevices.Buffer = L"\\Device\\ConDrv\\Console";
			return &u16_ntdevices;
		}
		if (strncmp(path + 5, "stdin", 6) == 0)
		{
			u16_ntdevices.Length = 48;
			u16_ntdevices.MaximumLength = u16_ntdevices.Length;
			u16_ntdevices.Buffer = L"\\Device\\ConDrv\\CurrentIn";
			return &u16_ntdevices;
		}
		if (strncmp(path + 5, "stdout", 7) == 0)
		{
			u16_ntdevices.Length = 50;
			u16_ntdevices.MaximumLength = u16_ntdevices.Length;
			u16_ntdevices.Buffer = L"\\Device\\ConDrv\\CurrentOut";
			return &u16_ntdevices;
		}
		if (strncmp(path + 5, "stderr", 7) == 0)
		{
			u16_ntdevices.Length = 50;
			u16_ntdevices.MaximumLength = u16_ntdevices.Length;
			u16_ntdevices.Buffer = L"\\Device\\ConDrv\\CurrentOut";
			return &u16_ntdevices;
		}
		// Other devices to be implemented via drivers. TODO
		if (strncmp(path + 5, "zero", 5) == 0)
		{
			u16_ntdevices.Length = 24;
			u16_ntdevices.MaximumLength = u16_ntdevices.Length;
			u16_ntdevices.Buffer = L"\\Device\\Zero";
			return &u16_ntdevices;
		}
		if (strncmp(path + 5, "full", 5) == 0)
		{
			u16_ntdevices.Length = 24;
			u16_ntdevices.MaximumLength = u16_ntdevices.Length;
			u16_ntdevices.Buffer = L"\\Device\\Full";
			return &u16_ntdevices;
		}
		if (strncmp(path + 5, "random", 7) == 0)
		{
			u16_ntdevices.Length = 28;
			u16_ntdevices.MaximumLength = u16_ntdevices.Length;
			u16_ntdevices.Buffer = L"\\Device\\Random";
			return &u16_ntdevices;
		}
		if (strncmp(path + 5, "urandom", 8) == 0)
		{
			u16_ntdevices.Length = 28;
			u16_ntdevices.MaximumLength = u16_ntdevices.Length;
			u16_ntdevices.Buffer = L"\\Device\\Random";
			return &u16_ntdevices;
		}
	}

	// DOS devices
	if (stricmp(path, "NUL") == 0)
	{
		u16_ntdevices.Length = 24;
		u16_ntdevices.MaximumLength = u16_ntdevices.Length;
		u16_ntdevices.Buffer = L"\\Device\\Null";
		return &u16_ntdevices;
	}
	if (strnicmp(path, "CON", 3) == 0)
	{

		if (path[3] == '\0')
		{
			u16_ntdevices.Length = 44;
			u16_ntdevices.MaximumLength = u16_ntdevices.Length;
			u16_ntdevices.Buffer = L"\\Device\\ConDrv\\Console";
			return &u16_ntdevices;
		}
		if (strnicmp(path + 3, "IN$", 4) == 0)
		{
			u16_ntdevices.Length = 48;
			u16_ntdevices.MaximumLength = u16_ntdevices.Length;
			u16_ntdevices.Buffer = L"\\Device\\ConDrv\\CurrentIn";
			return &u16_ntdevices;
		}
		if (strnicmp(path + 3, "OUT$", 5) == 0)
		{
			u16_ntdevices.Length = 50;
			u16_ntdevices.MaximumLength = u16_ntdevices.Length;
			u16_ntdevices.Buffer = L"\\Device\\ConDrv\\CurrentOut";
			return &u16_ntdevices;
		}
	}

	if ( // Normal Windows way -> C:
		(isalpha(path[0]) && path[1] == ':') ||
		// Cygwin way /c
		path[0] == '/')
	{
		path_is_absolute = true;
		if (path[0] == '/')
		{
			cygwin_path = true;
		}
	}
	else
	{
		// After this block u16_rootdir will be filled with a trailing slash and *without* a terminating NULL
		if (dirfd == AT_FDCWD)
		{
			/*
			   Calling `NtQueryObject` on the directory handle every time is slow.
			   There is no way of knowing whether the current working directory has changed
			   based on the handle and buffer as they are reused for subsequent `SetCurrentDirectory` calls.
			   We rely on the dospath and change the drive letter to it's NT device name. Since we cache the
			   devices this approach is faster and no overhead syscalls happen here (except for the very first call ofcourse).
			*/
			PUNICODE_STRING pu16_cwd;
			// TODO Locking
			pu16_cwd = &NtCurrentPeb()->ProcessParameters->CurrentDirectory.DosPath;
			// Eg "C:\Windows\"" -> "\Windows\"
			short cwd_length_without_drive_letter = pu16_cwd->Length - 2 * sizeof(WCHAR);
			rootdir_buffer = (char *)malloc(1024);

			// No need to check return value here as RTL_USER_PROCESS_PARAMETERS can be trusted
			device = dos_device_to_nt_device(pu16_cwd->Buffer[0]);

			memcpy(rootdir_buffer, device->name, device->length);
			memcpy(rootdir_buffer + device->length, pu16_cwd->Buffer + 2, cwd_length_without_drive_letter);

			u16_rootdir.Length = device->length + cwd_length_without_drive_letter;
			u16_rootdir.MaximumLength = u16_rootdir.Length;
			u16_rootdir.Buffer = (WCHAR *)rootdir_buffer;
		}
		else
		{
			rootdir_buffer = xget_fd_path(dirfd);
			if (rootdir_buffer == NULL)
			{
				// Bad file descriptor for directory.
				errno = EBADF;
				goto finish;
			}

			u16_rootdir.Length = ((PUNICODE_STRING)rootdir_buffer)->Length;
			u16_rootdir.MaximumLength = ((PUNICODE_STRING)rootdir_buffer)->MaximumLength;
			u16_rootdir.Buffer = ((PUNICODE_STRING)rootdir_buffer)->Buffer;
			// Add a trailing slash that overwrites the terminating NULL
			u16_rootdir.Buffer[u16_rootdir.Length / sizeof(WCHAR)] = L'\\';
			u16_rootdir.Length += 2;
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

	size_t required_size = u16_path.MaximumLength; // Includes L'\0'

	if (!path_is_absolute)
	{
		required_size += u16_rootdir.Length;
	}
	else // absolute path
	{
		required_size += 54; // "\\Device\\HarddiskVolumeXXXX\\"
	}

	ntpath_buffer = (WCHAR *)malloc(required_size);

	if (!path_is_absolute)
	{
		memcpy((char *)ntpath_buffer, u16_rootdir.Buffer, u16_rootdir.Length);
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
		memcpy((char *)ntpath_buffer + device->length, u16_path.Buffer + 2, u16_path.MaximumLength - 2 * sizeof(WCHAR));
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
	u16_ntpath->Buffer[u16_ntpath->Length / sizeof(WCHAR)] = L'\0';

finish:
	free(components);
	free(rootdir_buffer);
	free(ntpath_buffer);

	return u16_ntpath;
}

void free_ntpath(UNICODE_STRING *ntpath)
{
	// Hack to prevent free'ing constant strings for device names like 'NUL', 'CON'.
	if (ntpath == NULL || ntpath->Length == ntpath->MaximumLength)
	{
		return;
	}

	free(ntpath);
}
