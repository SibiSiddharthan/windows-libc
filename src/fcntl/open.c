/*
   Copyright (c) 2020-2022 Sibi Siddharthan

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

HANDLE really_do_open(OBJECT_ATTRIBUTES *object, ACCESS_MASK access, ULONG attributes, ULONG disposition, ULONG options)
{
	NTSTATUS status;
	IO_STATUS_BLOCK io;
	HANDLE handle = INVALID_HANDLE_VALUE;
	ULONG share = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;

	status = NtCreateFile(&handle, access, object, &io, NULL, attributes, share, disposition, options, NULL, 0);
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

	// Opening the console requires either FILE_GENERIC_READ or FILE_GENERIC_WRITE
	if (memcmp(ntpath->Buffer + 8, L"ConDrv", 12) == 0) // skip "\Device\"
	{
		options = FILE_SYNCHRONOUS_IO_NONALERT;
		if (access & (FILE_WRITE_DATA | FILE_WRITE_ATTRIBUTES | FILE_WRITE_EA))
		{
			access = FILE_GENERIC_WRITE;
		}
		else
		{
			access = FILE_GENERIC_READ;
		}
	}

	InitializeObjectAttributes(&object, ntpath, OBJ_CASE_INSENSITIVE, NULL, NULL);
	return really_do_open(&object, access, 0, FILE_OPEN, options);
}

HANDLE just_open(int dirfd, const char *path, ACCESS_MASK access, ULONG options)
{
	HANDLE handle = INVALID_HANDLE_VALUE;
	UNICODE_STRING *u16_ntpath = xget_absolute_ntpath(dirfd, path);

	if (u16_ntpath == NULL)
	{
		// errno will be set by `get_absolute_ntpath`.
		return handle;
	}

	handle = just_open2(u16_ntpath, access, options);
	free(u16_ntpath);

	return handle;
}

int do_open(int dirfd, const char *name, int oflags, mode_t perm)
{
	HANDLE handle;
	OBJECT_ATTRIBUTES object;
	ACCESS_MASK access_rights = determine_access_rights(oflags);
	ULONG attributes = 0;
	ULONG disposition = determine_create_dispostion(oflags);
	ULONG options = determine_create_options(oflags);
	PSECURITY_DESCRIPTOR security_descriptor = NULL;
	UNICODE_STRING *u16_ntpath = xget_absolute_ntpath(dirfd, name);

	int fd = -1;
	bool is_console = false;
	bool is_null = false;

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
		USHORT temp_bufsize = (1 + 6) * sizeof(WCHAR); // number of digits of 32bit random(5) + slash(1) + NULL
		UNICODE_STRING *u16_temppath = (UNICODE_STRING *)malloc(sizeof(UNICODE_STRING) + u16_ntpath->Length + temp_bufsize);

		u16_temppath->Buffer = (WCHAR *)((char *)u16_temppath + sizeof(UNICODE_STRING));
		memcpy(u16_temppath->Buffer, u16_ntpath->Buffer, u16_ntpath->Length);
		u16_temppath->Length = u16_ntpath->Length;
		u16_temppath->MaximumLength = u16_ntpath->Length + temp_bufsize;
		memset((char *)u16_temppath->Buffer + u16_temppath->Length, 0, temp_bufsize);

		// now swap the buffers
		free(u16_ntpath);
		u16_ntpath = u16_temppath;

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
		// Remember the 'Length' should always be less than 'MaximumLength' so that `free` will free the memory.
		memcpy((char *)u16_temppath->Buffer + u16_temppath->Length, rbuf,
			   u16_temppath->MaximumLength - u16_temppath->Length - sizeof(WCHAR));
		u16_ntpath->Length = u16_temppath->MaximumLength - sizeof(WCHAR);
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

	if (memcmp(u16_ntpath->Buffer + 8, L"Null", 10) == 0) // Include the NULL in comparison as well.
	{
		// NOTE: NtCreateFile on 'NUL' fails with STATUS_ACCESS_DENIED if requesting WRITE_DAC or WRITE_OWNER accesses.
		is_null = true;
		attributes = 0;
		options = FILE_SYNCHRONOUS_IO_NONALERT;
	}

	// opening console requires these
	if (memcmp(u16_ntpath->Buffer + 8, L"ConDrv", 12) == 0) // skip "\Device\"
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
		handle_t type;
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
			IO_STATUS_BLOCK io;
			FILE_ATTRIBUTE_TAG_INFORMATION tag_info;

			status = NtQueryInformationFile(handle, &io, &tag_info, sizeof(FILE_ATTRIBUTE_TAG_INFORMATION), FileAttributeTagInformation);
			if(status != STATUS_SUCCESS)
			{
				// Getting file attributes as failed. This means that we have not opened file or directory. (Maybe opened a device)
				// This should not happen at all.
				map_ntstatus_to_errno(status);
				NtClose(handle);
				goto finish;
			}

			if ((tag_info.FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) && ((oflags & O_NOFOLLOW) && ((oflags & O_PATH) == 0)))
			{
				// Close the handle if file is a symbolic link but only O_NOFOLLOW is specified. (Specify O_PATH also)
				NtClose(handle);
				errno = ELOOP;
				goto finish;
			}

			if ((access_rights & (FILE_WRITE_DATA | FILE_APPEND_DATA)) && (tag_info.FileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				// Close the handle if we request write access to a directory
				NtClose(handle);
				errno = EISDIR;
				goto finish;
			}

			if (tag_info.FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
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

		fd = register_to_fd_table(handle, type, oflags);
	}

finish:
	// u16_ntpath is malloc'ed, so free it.
	free(u16_ntpath);
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

	// glibc does not do bounds checking. Will set a bound for perm when ACLs are implemented
	// if (perm > 0777)
	//{
	//	errno = EINVAL;
	//	return -1;
	//}

	return do_open(dirfd, name, oflags, perm);
}
