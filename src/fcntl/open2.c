/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#define _CRT_RAND_S

#include <fcntl.h>
#include <errno.h>
#include <stdbool.h>
#include <internal/fcntl.h>
#include <internal/nt.h>
#include <internal/error.h>
#include <stdlib.h>
#include <sys/stat.h> // remove this
#include <errno.h>

static ACCESS_MASK determine_access_rights(int oflags)
{
	ACCESS_MASK access_rights = SYNCHRONIZE |                                        // always have this except for nonblocking handles
								FILE_READ_ATTRIBUTES | FILE_READ_EA | READ_CONTROL | // read
								FILE_WRITE_ATTRIBUTES | FILE_WRITE_EA | WRITE_DAC;   // write
	// Check if need to give this much by default. also WRITE_OWNER?

	if (oflags & O_WRONLY)
	{
		access_rights |= FILE_WRITE_DATA;
	}
	else if (oflags & O_RDWR)
	{
		access_rights |= FILE_READ_DATA | FILE_WRITE_DATA;
	}
	else // nothing is given i.e O_RDONLY by default
	{
		access_rights |= FILE_READ_DATA;
	}

	if (oflags & O_APPEND)
	{
		access_rights |= FILE_APPEND_DATA;
	}
	if (oflags & O_DIRECTORY)
	{
		// We don't technically need this as all users have BYPASS_TRAVERSE_CHECKING privilege, but just in case.
		access_rights |= FILE_TRAVERSE;
	}
	if (oflags & O_TMPFILE || oflags & O_SHORT_LIVED)
	{
		access_rights |= DELETE;
	}
	if (oflags & O_PATH)
	{
		// Only have synchronize, read_attributes, read_dac access if O_PATH is specified
		access_rights = SYNCHRONIZE | FILE_READ_ATTRIBUTES | READ_CONTROL;
	}

	return access_rights;
}

static ULONG determine_create_dispostion(int oflags)
{
	ULONG main_flags = oflags & (O_CREAT | O_EXCL | O_TRUNC);

	if (oflags & O_TMPFILE)
	{
		return FILE_CREATE;
	}

	switch (main_flags)
	{
	case 0x0:
	case O_EXCL:
		return FILE_OPEN;
	case O_CREAT:
		return FILE_OPEN_IF;
	case O_TRUNC:
	case O_TRUNC | O_EXCL:
		return FILE_OVERWRITE;
	case O_CREAT | O_EXCL:
	case O_CREAT | O_TRUNC | O_EXCL:
		return FILE_CREATE;
	case O_CREAT | O_TRUNC:
		return FILE_OVERWRITE_IF;
	}
	return -1ul; // unreachable
}

static ULONG determine_create_options(int oflags)
{
	ULONG options = FILE_SYNCHRONOUS_IO_NONALERT;
	if (oflags & O_TMPFILE || oflags & O_SHORT_LIVED)
	{
		options |= FILE_DELETE_ON_CLOSE | FILE_NON_DIRECTORY_FILE;
	}
	if (oflags & O_DIRECT)
	{
		options |= FILE_NO_INTERMEDIATE_BUFFERING;
	}
	if (oflags & O_SYNC)
	{
		options |= FILE_WRITE_THROUGH;
	}
	if (oflags & O_RANDOM)
	{
		options |= FILE_RANDOM_ACCESS;
	}
	if (oflags & O_SEQUENTIAL)
	{
		options |= FILE_SEQUENTIAL_ONLY;
	}
	if (oflags & O_NONBLOCK)
	{
		// options &= ~FILE_SYNCHRONOUS_IO_NONALERT; TODO
	}
	if (oflags & O_NOFOLLOW)
	{
		options |= FILE_OPEN_REPARSE_POINT;
	}
	if (oflags & O_DIRECTORY)
	{
		options |= FILE_DIRECTORY_FILE;
	}
	if (oflags & O_NOTDIR)
	{
		options |= FILE_NON_DIRECTORY_FILE;
	}
	return options;
}

static ULONG determine_file_attributes(int oflags)
{
	ULONG attributes = 0;
	if (oflags & O_TMPFILE)
	{
		attributes |= FILE_ATTRIBUTE_TEMPORARY;
	}
	return attributes;
}

static bool validate_oflags(int oflags)
{
	// A directory cannot be created with open, use mkdir
	if ((oflags & (O_CREAT | O_DIRECTORY)) == (O_CREAT | O_DIRECTORY))
	{
		return false;
	}

	if ((oflags & O_TMPFILE) && (oflags & (O_WRONLY | O_RDWR)) == 0)
	{
		return false;
	}

	return true;
}

static int path_components_size = 32;

typedef struct
{
	int start;  // starting offset
	int length; // length of component
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

wchar_t *get_absolute_ntpath(int dirfd, const char *path)
{
	wchar_t *u16_ntpath = NULL;
	wchar_t *u16_ntpath_final = NULL;
	wchar_t *rootdir = NULL;
	char *query_buffer = NULL;
	int length_root = 0;
	int length_path = 0;
	bool path_is_absolute = false;

	if (strncmp(path, "/dev/", 5) == 0)
	{
		if (strncmp(path + 5, "null", 5) == 0)
		{
			u16_ntpath_final = (wchar_t *)malloc(sizeof(wchar_t) * 8);
			memcpy(u16_ntpath_final, L"\\??\\NUL", 16);
			return u16_ntpath_final;
		}
		if (strncmp(path + 5, "tty", 4) == 0)
		{
			u16_ntpath_final = (wchar_t *)malloc(sizeof(wchar_t) * 8);
			memcpy(u16_ntpath_final, L"\\??\\CON", 16);
			return u16_ntpath_final;
		}
	}

	// dos devices
	if (stricmp(path, "NUL") == 0)
	{
		u16_ntpath_final = (wchar_t *)malloc(sizeof(wchar_t) * 8);
		memcpy(u16_ntpath_final, L"\\??\\NUL", 16);
		return u16_ntpath_final;
	}
	if (stricmp(path, "CON") == 0)
	{
		u16_ntpath_final = (wchar_t *)malloc(sizeof(wchar_t) * 8);
		memcpy(u16_ntpath_final, L"\\??\\CON", 16);
		return u16_ntpath_final;
	}

	if (isalpha(path[0]) && path[1] == ':')
	{
		path_is_absolute = true;
	}
	else
	{
		if (dirfd == AT_FDCWD)
		{
			rootdir = NtCurrentPeb()->ProcessParameters->CurrentDirectory.DosPath.Buffer;
			length_root = NtCurrentPeb()->ProcessParameters->CurrentDirectory.DosPath.Length;
		}
		else
		{
			// HANDLE rootdir_handle = get_fd_handle(dirfd);
			// ULONG return_length = 0;
			// IO_STATUS_BLOCK I;
			// query_buffer = (char *)malloc(1024);
			// NTSTATUS status = NtQueryObject(rootdir_handle, ObjectNameInformation, query_buffer, 1024, &return_length);
			// POBJECT_NAME_INFORMATION name = (POBJECT_NAME_INFORMATION)query_buffer;
			// length_root = name->Name.Length;
			// rootdir = name->Name.Buffer;
			rootdir = (wchar_t *)get_fd_path(dirfd);
			length_root = wcslen(rootdir) * sizeof(wchar_t);
		}
	}

	UTF8_STRING u8_path;
	UNICODE_STRING u16_path;
	RtlInitUTF8String(&u8_path, path);
	RtlUTF8StringToUnicodeString(&u16_path, &u8_path, TRUE);
	length_path = u16_path.Length;
	u16_ntpath = (wchar_t *)malloc(length_path + length_root + 8 + 2); // 8 -> \??\ , 2 -> NULL

	memcpy(u16_ntpath, L"\\??\\", 8); // optimize these loads
	if (!path_is_absolute)
	{
		memcpy((char *)u16_ntpath + 8, rootdir, length_root);
	}
	memcpy((char *)u16_ntpath + 8 + length_root, u16_path.Buffer, length_path);
	u16_ntpath[(8 + length_root + length_path) / sizeof(wchar_t)] = L'\0';

	RtlFreeUnicodeString(&u16_path);

	// Convert forward slashes to backward slashes
	for (int i = 4 + length_root / sizeof(wchar_t); u16_ntpath[i] != L'\0'; i++) // skip \??\ and root path
	{
		if (u16_ntpath[i] == L'/')
		{
			u16_ntpath[i] = L'\\';
		}
	}

	path_component *components = NULL;

	int index = 0;
	int start = 0;
	for (int i = 6;; i++) // start after \??\C:
	{
		if (u16_ntpath[i] == L'\\' || u16_ntpath[i] == L'\0')
		{
			if (i - start > 2) // not '.' or '..'
			{
				components = add_component(components, &index, start, i - start);
			}
			else
			{
				if (i - start == 2 && memcmp(u16_ntpath + start, L"..", 4) == 0)
				{
					--index;
					if (index < 1) // bad path
					{
						errno = ENOENT;
						free(query_buffer);
						free(components);
						free(u16_ntpath);
						free(u16_ntpath_final);
						return NULL;
					}
				}
				else if (i - start == 1 && memcmp(u16_ntpath + start, L".\\", 2) == 0)
				{
					; // do nothing
				}
				else
				{
					components = add_component(components, &index, start, i - start);
				}
			}

			if (u16_ntpath[i] == L'\0')
			{
				break;
			}
			start = i + 1;
		}
	}

	u16_ntpath_final = (wchar_t *)malloc(length_path + length_root + 8 + 2); // 8 -> \??\ , 2 -> NULL
	// memcpy(u16_ntpath_final, u16_ntpath, root.length * sizeof(wchar_t));
	int final_length = 0; // root.length;
	for (int i = 0; i < index; i++)
	{
		memcpy(u16_ntpath_final + final_length, u16_ntpath + components[i].start, components[i].length * sizeof(wchar_t));
		final_length += components[i].length;
		if (i + 1 < index)
		{
			u16_ntpath_final[final_length++] = L'\\';
		}
	}
	u16_ntpath_final[final_length++] = L'\0';

#if 0
	int total_length = (8 + 2 + 2 + length_path + length_root) / sizeof(wchar_t);
	for (int i = 4; u16_ntpath[i] != L'\0'; i++)
	{
		if (u16_ntpath[i] == L'.')
		{
			if (u16_ntpath[i - 1] == L'\\' && u16_ntpath[i + 1] == L'\0') // somepath\. trailing .
			{
				u16_ntpath[i - 1] = L'\0';
				break; // We have reached the end, break the loop
			}

			else if (i + 2 < total_length && u16_ntpath[i - 1] == L'\\' && u16_ntpath[i + 1] == L'.' &&
					 u16_ntpath[i + 2] == L'\0') // somepath\. trailing .
			{
				u16_ntpath[i - 1] = L'\0';
				break; // We have reached the end, break the loop
			}

			else if (u16_ntpath[i - 1] == L'\\' && u16_ntpath[i + 1] == L'\\') // somepath\.\file
			{
			}
		}
	}
#endif
	free(query_buffer);
	free(components);
	free(u16_ntpath);

	return u16_ntpath_final;
}

HANDLE really_do_open(OBJECT_ATTRIBUTES *object, ACCESS_MASK access, ULONG attributes, ULONG disposition, ULONG options)
{
	HANDLE H = INVALID_HANDLE_VALUE;
	IO_STATUS_BLOCK I;
	ULONG share = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
	NTSTATUS status = NtCreateFile(&H, access, object, &I, NULL, attributes, share, disposition, options, NULL, 0);
	if (status != STATUS_SUCCESS)
	{
		if (status == STATUS_OBJECT_NAME_INVALID)
		{
			int i;
			for (i = 6; object->ObjectName->Buffer[i] != L'\0'; i++) // skip \??\C:
			{
				wchar_t wc = object->ObjectName->Buffer[i];
				if (wc == L':' || wc == L'<' || wc == L'>' || wc == L'*' || wc == L'|' || wc == L'?' || wc == L'\"')
				{
					errno = EINVAL;
					return H;
				}
			}
			if (object->ObjectName->Buffer[i - 1] == L'\\')
			{
				errno = EISDIR;
				return H;
			}
		}
		map_ntstatus_to_errno(status);
	}
	return H;
}

HANDLE just_open(const wchar_t *u16_ntpath, ACCESS_MASK access, ULONG attributes, ULONG disposition, ULONG options)
{
	UNICODE_STRING u16_path;
	// Opening the console requires either FILE_GENERIC_READ or FILE_GENERIC_WRITE
	if (memcmp(u16_ntpath, L"\\??\\CON", 16) == 0)
	{
		attributes = 0;
		options = FILE_SYNCHRONOUS_IO_NONALERT;

		if (access & (FILE_WRITE_DATA | FILE_WRITE_ATTRIBUTES | FILE_WRITE_EA | WRITE_DAC))
		{
			access = FILE_GENERIC_WRITE;
		}
		else
		{
			access = FILE_GENERIC_READ;
		}
	}
	RtlInitUnicodeString(&u16_path, u16_ntpath);
	OBJECT_ATTRIBUTES object;
	InitializeObjectAttributes(&object, &u16_path, OBJ_CASE_INSENSITIVE, NULL, NULL);
	return really_do_open(&object, access, attributes, disposition, options);
}

int do_open(int dirfd, const char *name, int oflags, mode_t perm)
{
	int fd = -1;
	HANDLE handle;
	ACCESS_MASK access_rights = determine_access_rights(oflags);
	ULONG attributes = determine_file_attributes(oflags);
	ULONG disposition = determine_create_dispostion(oflags);
	ULONG options = determine_create_options(oflags);
	bool is_console = false;
	bool is_null = false;

	if (oflags & (O_CREAT | O_TMPFILE))
	{
		if ((perm & (S_IWRITE)) == 0)
		{
			attributes |= FILE_ATTRIBUTE_READONLY;
		}
	}

	wchar_t *u16_ntpath = get_absolute_ntpath(dirfd, name);

	if (oflags & O_TMPFILE)
	{
		int length = wcslen(u16_ntpath) + 1; // L'\0'
		wchar_t *temp = (wchar_t *)malloc(sizeof(wchar_t) * length);
		memcpy(temp, u16_ntpath, sizeof(wchar_t) * length);
		u16_ntpath = (wchar_t *)realloc(u16_ntpath, sizeof(wchar_t) * (length + 6)); // number of digits of 32bit random(5) + slash(1)
		memcpy(u16_ntpath, temp, sizeof(wchar_t) * length);
		memset(u16_ntpath + length, 0, sizeof(wchar_t) * 6);
		free(temp);

		if (u16_ntpath[length - 2] != L'\\')
		{
			u16_ntpath[length++ - 1] = L'\\';
		}

		// Generate a random number, convert it to a wchar_t string with base 36(0-9,a-z)
		unsigned int rn;
		rand_s(&rn);
		wchar_t rbuf[8] = {0, 0, 0, 0, 0, 0, 0, 0};
		_ultow_s(rn, rbuf, 8, 36);
		// Append the string to u16_ntpath
		wcsncat(u16_ntpath, rbuf, 5);
	}

	UNICODE_STRING u16_path;
	RtlInitUnicodeString(&u16_path, u16_ntpath);

	OBJECT_ATTRIBUTES object;
	InitializeObjectAttributes(&object, &u16_path, OBJ_CASE_INSENSITIVE | OBJ_INHERIT, NULL, NULL);
	if (oflags & O_NOINHERIT)
	{
		object.Attributes &= ~OBJ_INHERIT;
	}

	if (memcmp(u16_ntpath, L"\\??\\NUL", 16) == 0)
	{
		is_null = true;
		attributes = 0;
		options = FILE_SYNCHRONOUS_IO_NONALERT;
		// remove these or else NtCreateFile fails with STATUS_ACCESS_DENIED
		access_rights &= ~(WRITE_DAC | WRITE_OWNER);
	}

	// opening console requires these
	if (memcmp(u16_ntpath, L"\\??\\CON", 16) == 0)
	{
		is_console = true;
		attributes = 0;
		options = FILE_SYNCHRONOUS_IO_NONALERT;

		if (oflags & O_RDWR)
		{
			errno = EINVAL;
			goto finish;
		}
		else if (oflags & O_WRONLY)
		{
			access_rights = FILE_GENERIC_WRITE;
		}
		else
		{
			access_rights = FILE_GENERIC_READ;
		}
	}

	handle = really_do_open(&object, access_rights, attributes, disposition, options);

	if (handle != INVALID_HANDLE_VALUE)
	{
		enum handle_type type;
		// Check the pathname for identifying these, otherwise we would need to NtQueryVolumeInformationFIle
		if (is_console || is_null)
		{
			if (is_console)
			{
				type = CONSOLE_HANDLE;
			}
			else // is_null
			{
				type = NULL_HANDLE;
			}
		}
		else
		{
			// Handle belongs to a disk file, find out whether it is a file or directory
			NTSTATUS status;
			IO_STATUS_BLOCK I;
			FILE_ATTRIBUTE_TAG_INFORMATION INFO;

			status = NtQueryInformationFile(handle, &I, &INFO, sizeof(FILE_ATTRIBUTE_TAG_INFORMATION), FileAttributeTagInformation);

			if ((INFO.FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) && ((oflags & O_NOFOLLOW) && ((oflags & O_PATH) == 0)))
			{
				// Close the handle if file is a symbolic link but only O_NOFOLLOW is specified. (Specify O_PATH also)
				NtClose(handle);
				errno = ELOOP;
				goto finish;
			}

			if ((access_rights & (FILE_WRITE_DATA | FILE_APPEND_DATA)) && (INFO.FileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				// Close the handle if we request write access to a directory
				NtClose(handle);
				errno = EISDIR;
				goto finish;
			}

			if (INFO.FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				type = DIRECTORY_HANDLE;
			}
			else
			{
				type = FILE_HANDLE;
			}

			if (oflags & O_NOATIME && (oflags & O_PATH) == 0)
			{
				FILE_BASIC_INFO BASIC_INFO;
				status = NtQueryInformationFile(handle, &I, &BASIC_INFO, sizeof(FILE_BASIC_INFO), FileBasicInformation);
				if (status == STATUS_SUCCESS)
				{
					// Set it to -1 for no updates
					BASIC_INFO.LastAccessTime.QuadPart = -1;
					status = NtSetInformationFile(handle, &I, &BASIC_INFO, sizeof(FILE_BASIC_INFO), FileBasicInformation);
					if (status != STATUS_SUCCESS)
					{
						map_ntstatus_to_errno(status); // just set errno;
					}
				}
				else
				{
					map_ntstatus_to_errno(status); // just set errno;
				}
			}
		}

		fd = register_to_fd_table(handle, u16_ntpath + 4, type, oflags); // skip "\??\"
	}

finish:
	// u16_ntpath is malloc'ed, so free it.
	free(u16_ntpath);
	return fd;
}

#if 0

int open_tty(const int oflags)
{
	HANDLE H;
	OBJECT_ATTRIBUTES O;
	IO_STATUS_BLOCK I;
	UNICODE_STRING S;
	InitializeObjectAttributes(&O, &S, OBJ_CASE_INSENSITIVE | OBJ_INHERIT, NULL, NULL);
	if (oflags & O_NOINHERIT)
	{
		O.Attributes &= ~OBJ_INHERIT;
	}
	ACCESS_MASK A = GENERIC_READ | GENERIC_WRITE; // this requires the generic access constants
	if (oflags & O_WRONLY)
	{
		RtlInitUnicodeString(&S, L"\\??\\CONOUT$");
	}
	else
	{
		RtlInitUnicodeString(&S, L"\\??\\CONIN$");
	}

	NTSTATUS STATUS =
		NtCreateFile(&H, A, &O, &I, NULL, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, FILE_OPEN, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
}
#endif



int wlibc_common_open(int dirfd, const char *name, int oflags, va_list perm_args)
{
	VALIDATE_PATH_AND_DIRFD(name, dirfd);

	if (!validate_oflags(oflags))
	{
		errno = EINVAL;
		return -1;
	}

	mode_t perm = 0;
	if (oflags & (O_CREAT | O_TMPFILE))
	{
		perm = va_arg(perm_args, mode_t);
	}

	if (perm > 0777)
	{
		errno = EINVAL;
		return -1;
	}

	return do_open(dirfd, name, oflags, perm);
}

#if 0
int wlibc_openat2(int dirfd, const char *name, int oflags, ...)
{
	va_list perm_args;
	va_start(perm_args, oflags);
	int fd = wlibc_common_open(dirfd, name, oflags, perm_args);
	va_end(perm_args);
	return fd;
}

int wlibc_open2(const char *name, int oflags, ...)
{
	va_list perm_args;
	va_start(perm_args, oflags);
	int fd = wlibc_common_open(AT_FDCWD, name, oflags, perm_args);
	va_end(perm_args);
	return fd;
}
#endif
