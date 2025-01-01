/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <internal/fcntl.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/statvfs.h>

// From statvfs.c
unsigned long get_fs_type(const char *name);

static unsigned long get_fs_flags(DWORD attributes)
{
	unsigned long flags = ST_NOSUID | ST_SYNCHRONOUS | ST_MANDLOCK;

	if (attributes & FILE_READ_ONLY_VOLUME)
	{
		flags |= ST_RDONLY;
	}

	return flags;
}

int do_statvfs(HANDLE handle, struct statvfs *restrict statvfsbuf)
{
	NTSTATUS status;
	IO_STATUS_BLOCK io;
	FILE_FS_FULL_SIZE_INFORMATION fs_info;
	FILE_FS_ATTRIBUTE_INFORMATION *attribute_info;
	FILE_FS_VOLUME_INFORMATION *volume_info;
	UNICODE_STRING u16_fstype;
	UTF8_STRING u8_fstype;
	char attribute_info_buffer[64];
	char volume_info_buffer[128];

	// In Windows there are no inodes. Ignore these fields.
	statvfsbuf->f_files = -1ull;
	statvfsbuf->f_ffree = -1ull;
	statvfsbuf->f_favail = -1ull;

	// Max path length on Windows.
	statvfsbuf->f_namemax = 32768;

	status = NtQueryVolumeInformationFile(handle, &io, &fs_info, sizeof(FILE_FS_FULL_SIZE_INFORMATION), FileFsFullSizeInformation);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	statvfsbuf->f_bsize = fs_info.BytesPerSector * fs_info.SectorsPerAllocationUnit;
	statvfsbuf->f_frsize = statvfsbuf->f_bsize; // Treate these two fields as the same.
	statvfsbuf->f_blocks = fs_info.TotalAllocationUnits.QuadPart;
	statvfsbuf->f_bfree = fs_info.ActualAvailableAllocationUnits.QuadPart;
	statvfsbuf->f_bavail = fs_info.CallerAvailableAllocationUnits.QuadPart;

	memset(attribute_info_buffer, 0, sizeof(attribute_info_buffer));
	attribute_info = (PFILE_FS_ATTRIBUTE_INFORMATION)attribute_info_buffer;

	status = NtQueryVolumeInformationFile(handle, &io, attribute_info, sizeof(attribute_info_buffer), FileFsAttributeInformation);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	if (attribute_info->FileSystemNameLength != 0)
	{
		// Copy the filesystem name to statvfsbuf->f_fstypename.
		u16_fstype.Buffer = attribute_info->FileSystemName;
		u16_fstype.Length = (USHORT)attribute_info->FileSystemNameLength;
		u16_fstype.MaximumLength = (USHORT)attribute_info->FileSystemNameLength;

		u8_fstype.Buffer = statvfsbuf->f_fstypename;
		u8_fstype.Length = 0;
		u8_fstype.MaximumLength = VFSTYPENAMELEN;

		RtlUnicodeStringToUTF8String(&u8_fstype, &u16_fstype, FALSE);
	}

	statvfsbuf->f_flag = get_fs_flags(attribute_info->FileSystemAttributes);
	statvfsbuf->f_type = get_fs_type(statvfsbuf->f_fstypename);

	volume_info = (PFILE_FS_VOLUME_INFORMATION)volume_info_buffer;
	status = NtQueryVolumeInformationFile(handle, &io, volume_info, sizeof(volume_info_buffer), FileFsVolumeInformation);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	statvfsbuf->f_fsid = volume_info->VolumeSerialNumber;

	return 0;
}

int wlibc_common_statvfs(int fd, const char *restrict path, struct statvfs *restrict statvfsbuf)
{
	if (statvfsbuf == NULL)
	{
		errno = EINVAL;
		return -1;
	}

	if ((fd == -1 && path == NULL) || (fd > -1 && (path != NULL && path[0] != '\0')))
	{
		errno = EINVAL;
		return -1;
	}

	if (fd > -1)
	{
		fdinfo info;

		get_fdinfo(fd, &info);

		if (info.type != FILE_HANDLE && info.type != DIRECTORY_HANDLE)
		{
			errno = EBADF;
			return -1;
		}

		return do_statvfs(info.handle, statvfsbuf);
	}
	else
	{
		int result;
		HANDLE handle;

		handle = just_open(AT_FDCWD, path, FILE_READ_ATTRIBUTES, 0);
		if (handle == NULL)
		{
			// errno will be set by `just_open`
			return -1;
		}

		result = do_statvfs(handle, statvfsbuf);
		NtClose(handle);

		return result;
	}
}
