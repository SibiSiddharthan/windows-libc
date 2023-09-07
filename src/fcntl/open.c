/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#define _CRT_RAND_S

#include <internal/nt.h>
#include <internal/error.h>
#include <internal/fcntl.h>
#include <internal/path.h>
#include <internal/security.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>

static ACCESS_MASK determine_access_rights(int oflags)
{
	ACCESS_MASK access_rights = SYNCHRONIZE |                                        // always have this except for nonblocking handles
								FILE_READ_ATTRIBUTES | FILE_READ_EA | READ_CONTROL | // read
								FILE_WRITE_ATTRIBUTES;                               // write

	if (oflags & O_WRONLY)
	{
		access_rights |= FILE_WRITE_DATA | FILE_WRITE_EA;
	}
	else if (oflags & O_RDWR)
	{
		access_rights |= FILE_READ_DATA | FILE_WRITE_DATA | FILE_WRITE_EA;
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
	if (oflags & O_READONLY)
	{
		attributes |= FILE_ATTRIBUTE_READONLY;
	}
	if (oflags & O_HIDDEN)
	{
		attributes |= FILE_ATTRIBUTE_HIDDEN;
	}
	if (oflags & O_SYSTEM)
	{
		attributes |= FILE_ATTRIBUTE_SYSTEM;
	}
	if (oflags & O_ARCHIVE)
	{
		attributes |= FILE_ATTRIBUTE_ARCHIVE;
	}
	if (oflags & O_ENCRYPTED)
	{
		attributes |= FILE_ATTRIBUTE_ENCRYPTED;
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

static HANDLE do_reopen(HANDLE old_handle, ACCESS_MASK access, ULONG attributes, ULONG disposition, ULONG options)
{
	NTSTATUS status;
	IO_STATUS_BLOCK io;
	OBJECT_ATTRIBUTES object;
	UNICODE_STRING empty = {0, 0, NULL};
	HANDLE new_handle = NULL;
	ULONG share = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;

	// NOTE: An empty UNICODE_STRING needs to passed, for this to work.
	InitializeObjectAttributes(&object, &empty, 0, old_handle, NULL);

	// We can reopen the same file by passing its already open handle to the root parameter of OBJECT_ATTRIBUTES.
	status = NtCreateFile(&new_handle, access, &object, &io, NULL, attributes, share, disposition, options, NULL, 0);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
	}

	return new_handle;
}

HANDLE just_reopen(HANDLE handle, ACCESS_MASK access, ULONG options)
{
	return do_reopen(handle, access, 0, FILE_OPEN, options);
}

HANDLE reopen_handle(HANDLE handle, int flags)
{
	ACCESS_MASK access = determine_access_rights(flags);
	ULONG options = determine_create_options(flags);
	ULONG disposition = determine_create_dispostion(flags);
	ULONG attributes = determine_file_attributes(flags);

	return do_reopen(handle, access, attributes, disposition, options);
}

static HANDLE really_do_open(OBJECT_ATTRIBUTES *object, ACCESS_MASK access, ULONG attributes, ULONG disposition, ULONG options)
{
	NTSTATUS status;
	IO_STATUS_BLOCK io;
	HANDLE handle = NULL;
	ULONG share = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;

	status = NtCreateFile(&handle, access, object, &io, NULL, attributes, share, disposition, options, NULL, 0);
	if (status != STATUS_SUCCESS)
	{
		if (status == STATUS_OBJECT_NAME_INVALID)
		{
			int i;
			for (i = 22; object->ObjectName->Buffer[i] != L'\0'; i++) // skip "\Device\HarddiskVolume"
			{
				wchar_t wc = object->ObjectName->Buffer[i];
				if (wc == L':' || wc == L'<' || wc == L'>' || wc == L'*' || wc == L'|' || wc == L'?' || wc == L'\"')
				{
					errno = EINVAL;
					return handle;
				}
			}
			if (object->ObjectName->Buffer[i - 1] == L'\\')
			{
				errno = EISDIR;
				return handle;
			}
		}
		map_ntstatus_to_errno(status);
	}
	return handle;
}

HANDLE just_open2(UNICODE_STRING *ntpath, ACCESS_MASK access, ULONG options)
{
	OBJECT_ATTRIBUTES object;

	InitializeObjectAttributes(&object, ntpath, OBJ_CASE_INSENSITIVE, NULL, NULL);
	return really_do_open(&object, access, 0, FILE_OPEN, options);
}

HANDLE just_open(int dirfd, const char *path, ACCESS_MASK access, ULONG options)
{
	handle_t type;
	HANDLE handle = NULL;
	UNICODE_STRING *u16_ntpath = get_absolute_ntpath2(dirfd, path, &type);

	if (u16_ntpath == NULL)
	{
		// errno will be set by `get_absolute_ntpath2`.
		return handle;
	}

	// Opening the console requires either FILE_GENERIC_READ or FILE_GENERIC_WRITE
	if (type == CONSOLE_HANDLE)
	{
		options = FILE_SYNCHRONOUS_IO_NONALERT;
		if (access & (FILE_WRITE_DATA | FILE_APPEND_DATA | FILE_WRITE_ATTRIBUTES | FILE_WRITE_EA))
		{
			access = FILE_GENERIC_WRITE;
		}
		else //(access & (FILE_READ_DATA | FILE_READ_ATTRIBUTES | FILE_READ_EA))
		{
			access = FILE_GENERIC_READ;
		}
	}

	handle = just_open2(u16_ntpath, access, options);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, u16_ntpath);

	return handle;
}

HANDLE do_os_open(int dirfd, const char *name, int oflags, mode_t perm, handle_t *type)
{
	HANDLE handle = NULL;
	OBJECT_ATTRIBUTES object;
	ACCESS_MASK access_rights = determine_access_rights(oflags);
	ULONG attributes = 0;
	ULONG disposition = determine_create_dispostion(oflags);
	ULONG options = determine_create_options(oflags);
	PSECURITY_DESCRIPTOR security_descriptor = NULL;
	UNICODE_STRING *u16_ntpath = get_absolute_ntpath2(dirfd, name, type);

	if (u16_ntpath == NULL)
	{
		goto finish;
	}

	if (oflags & (O_CREAT | O_TMPFILE))
	{
#if 0
		if ((perm & (S_IWRITE)) == 0)
		{
			attributes |= FILE_ATTRIBUTE_READONLY;
		}
#endif
		attributes |= determine_file_attributes(oflags);
		security_descriptor = (PSECURITY_DESCRIPTOR)get_security_descriptor(perm & 0777, 0);
	}

	if (oflags & O_TMPFILE)
	{
		USHORT rand_bufsize = (1 + 6) * sizeof(WCHAR); // number of digits of 32bit random(5) + slash(1) + NULL
		UNICODE_STRING *u16_temppath = (UNICODE_STRING *)RtlReAllocateHeap(NtCurrentProcessHeap(), HEAP_ZERO_MEMORY, u16_ntpath,
																		   sizeof(UNICODE_STRING) + u16_ntpath->Length + rand_bufsize);

		if (u16_temppath == NULL)
		{
			goto finish;
		}

		u16_ntpath = u16_temppath;

		// Realloc will preserve the contents in memory. Update the buffer pointer, maximum length.
		u16_ntpath->Buffer = (WCHAR *)((char *)u16_ntpath + sizeof(UNICODE_STRING));
		u16_ntpath->MaximumLength = u16_ntpath->Length + rand_bufsize; 

		if (u16_ntpath->Buffer[u16_ntpath->Length / sizeof(WCHAR) - 1] != L'\\')
		{
			u16_ntpath->Buffer[u16_ntpath->Length / sizeof(WCHAR)] = L'\\';
			u16_ntpath->Length += sizeof(WCHAR);
		}

		// Generate a random number, convert it to a wchar_t string with base 36(0-9,a-z)
		unsigned int rn;
		rand_s(&rn);
		wchar_t rbuf[8] = {0, 0, 0, 0, 0, 0, 0, 0};
		_ultow_s(rn, rbuf, 8, 36);

		// Append the string to u16_ntpath.
		memcpy((char *)u16_ntpath->Buffer + u16_ntpath->Length, rbuf, u16_ntpath->MaximumLength - u16_ntpath->Length - sizeof(WCHAR));
		u16_ntpath->Length = u16_ntpath->MaximumLength - sizeof(WCHAR);

		// Start from the end to figure out the length.
		while (u16_ntpath->Buffer[u16_ntpath->Length / sizeof(WCHAR) - 1] == L'\0')
		{
			u16_ntpath->Length -= sizeof(WCHAR);
		}
	}

	InitializeObjectAttributes(&object, u16_ntpath, OBJ_CASE_INSENSITIVE | OBJ_INHERIT, NULL, security_descriptor);
	if (oflags & O_NOINHERIT)
	{
		object.Attributes &= ~OBJ_INHERIT;
	}

	if (*type == NULL_HANDLE) // Include the NULL in comparison as well.
	{
		attributes = 0;
		options = FILE_SYNCHRONOUS_IO_NONALERT;
	}

	// Opening console requires these.
	if (*type == CONSOLE_HANDLE) // skip "\Device\"
	{
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

	if (handle != NULL)
	{
		if (*type == FILE_HANDLE) // Type set by `get_absolute_ntpath2` when file to be opened is on disk.
		{
			// Handle belongs to a disk file, find out whether it is a file or directory
			NTSTATUS status;
			IO_STATUS_BLOCK io;
			FILE_ATTRIBUTE_TAG_INFORMATION tag_info;

			status = NtQueryInformationFile(handle, &io, &tag_info, sizeof(FILE_ATTRIBUTE_TAG_INFORMATION), FileAttributeTagInformation);
			if (status != STATUS_SUCCESS)
			{
				// Getting file attributes as failed. This means that we have not opened file or directory. (Maybe opened a device)
				// This should not happen at all.
				map_ntstatus_to_errno(status);
				NtClose(handle);
				handle = NULL;
				goto finish;
			}

			if ((tag_info.FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) && ((oflags & O_NOFOLLOW) && ((oflags & O_PATH) == 0)))
			{
				// Close the handle if file is a symbolic link but only O_NOFOLLOW is specified. (Specify O_PATH also)
				NtClose(handle);
				handle = NULL;
				errno = ELOOP;
				goto finish;
			}

			if ((access_rights & (FILE_WRITE_DATA | FILE_APPEND_DATA)) && (tag_info.FileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				// Close the handle if we request write access to a directory
				NtClose(handle);
				handle = NULL;
				errno = EISDIR;
				goto finish;
			}

			if (tag_info.FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				*type = DIRECTORY_HANDLE;
			}

			if (oflags & O_NOATIME && (oflags & O_PATH) == 0)
			{
				FILE_BASIC_INFO BASIC_INFO;
				status = NtQueryInformationFile(handle, &io, &BASIC_INFO, sizeof(FILE_BASIC_INFO), FileBasicInformation);
				if (status == STATUS_SUCCESS)
				{
					// Set it to -1 for no updates
					BASIC_INFO.LastAccessTime.QuadPart = -1;
					status = NtSetInformationFile(handle, &io, &BASIC_INFO, sizeof(FILE_BASIC_INFO), FileBasicInformation);
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
	}

finish:
	// u16_ntpath is malloc'ed, so free it.
	RtlFreeHeap(NtCurrentProcessHeap(), 0, u16_ntpath);
	return handle;
}

int do_open(int dirfd, const char *name, int oflags, mode_t perm)
{
	int fd = -1;
	handle_t type = INVALID_HANDLE;
	HANDLE handle = do_os_open(dirfd, name, oflags, perm, &type);

	if(handle != NULL)
	{	
		fd = register_to_fd_table(handle, type, oflags);
	}

	return fd;
}

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

	// Glibc does not do bounds checking for permissions.
	// if (perm > 0777)
	//{
	//	errno = EINVAL;
	//	return -1;
	//}

	return do_open(dirfd, name, oflags, perm);
}
