/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <internal/fcntl.h>
#include <internal/path.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/param.h>
#include <sys/statfs.h>

static unsigned long get_fs_flags(DWORD attributes)
{
	unsigned long flags = MNT_NOSUID | MNT_STRICTTIME | MNT_ASYNC | MNT_SYNCHRONOUS;

	if (attributes & FILE_READ_ONLY_VOLUME)
	{
		flags |= MNT_RDONLY;
	}
	if (attributes & FILE_VOLUME_QUOTAS)
	{
		flags |= MNT_QUOTA;
	}
	if (attributes & FILE_SUPPORTS_USN_JOURNAL)
	{
		attributes |= MNT_JOURNALED;
	}

	return flags;
}

unsigned long get_fs_type(const char *name)
{
	// Put NTFS first as it the most commonly used one.
	if (stricmp(name, "NTFS") == 0)
	{
		return NTFS_FS;
	}
	if (stricmp(name, "FAT32") == 0)
	{
		return FAT32_FS;
	}
	if (stricmp(name, "EXFAT") == 0)
	{
		return EXFAT_FS;
	}
	if (stricmp(name, "HPFS") == 0)
	{
		return HPFS_FS;
	}
	if (stricmp(name, "REFS") == 0)
	{
		return REFS_FS;
	}

	return UNKNOWN_FS;
}

int do_statfs(HANDLE handle, struct statfs *restrict statfsbuf)
{
	NTSTATUS status;
	IO_STATUS_BLOCK io;
	FILE_FS_FULL_SIZE_INFORMATION fs_info;
	FILE_FS_SECTOR_SIZE_INFORMATION sector_info;
	FILE_FS_ATTRIBUTE_INFORMATION *attribute_info;
	FILE_FS_VOLUME_INFORMATION *volume_info;
	UNICODE_STRING u16_fstype;
	UTF8_STRING u8_fstype;
	UNICODE_STRING *u16_ntpath = NULL, *u16_dospath = NULL;
	char attribute_info_buffer[64];
	char volume_info_buffer[128];

	memset(statfsbuf, 0, sizeof(struct statfs));

	// In Windows there are no inodes. Ignore these fields.
	statfsbuf->f_files = -1ull;
	statfsbuf->f_ffree = -1ull;
	statfsbuf->f_favail = -1ull;

	// Max path length on Windows.
	statfsbuf->f_namemax = MAXPATHLEN;

	// Only Administrator can mount drives.
	statfsbuf->f_owner = 0;

	status = NtQueryVolumeInformationFile(handle, &io, &fs_info, sizeof(FILE_FS_FULL_SIZE_INFORMATION), FileFsFullSizeInformation);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	statfsbuf->f_bsize = fs_info.BytesPerSector * fs_info.SectorsPerAllocationUnit;
	statfsbuf->f_blocks = fs_info.TotalAllocationUnits.QuadPart;
	statfsbuf->f_bfree = fs_info.ActualAvailableAllocationUnits.QuadPart;
	statfsbuf->f_bavail = fs_info.CallerAvailableAllocationUnits.QuadPart;

	status = NtQueryVolumeInformationFile(handle, &io, &sector_info, sizeof(FILE_FS_SECTOR_SIZE_INFORMATION), FileFsSectorSizeInformation);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	statfsbuf->f_iosize = sector_info.PhysicalBytesPerSectorForPerformance;

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
		// Copy the filesystem name to statfsbuf->f_fstypename.
		u16_fstype.Buffer = attribute_info->FileSystemName;
		u16_fstype.Length = (USHORT)attribute_info->FileSystemNameLength;
		u16_fstype.MaximumLength = (USHORT)attribute_info->FileSystemNameLength;

		u8_fstype.Buffer = statfsbuf->f_fstypename;
		u8_fstype.Length = 0;
		u8_fstype.MaximumLength = MFSTYPENAMELEN;

		RtlUnicodeStringToUTF8String(&u8_fstype, &u16_fstype, FALSE);
	}

	statfsbuf->f_flag = get_fs_flags(attribute_info->FileSystemAttributes);
	statfsbuf->f_type = get_fs_type(statfsbuf->f_fstypename);
	statfsbuf->f_fssubtype = 0; // We don't have any variations of filesystems.

	volume_info = (PFILE_FS_VOLUME_INFORMATION)volume_info_buffer;
	status = NtQueryVolumeInformationFile(handle, &io, volume_info, sizeof(volume_info_buffer), FileFsVolumeInformation);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	statfsbuf->f_fsid.major = (USHORT)(volume_info->VolumeSerialNumber >> 16);
	statfsbuf->f_fsid.minor = (USHORT)volume_info->VolumeSerialNumber;

	u16_ntpath = get_handle_ntpath(handle);
	if (u16_ntpath == NULL)
	{
		// errno will be set by `get_handle_ntpath`.
		return -1;
	}

	memcpy(statfsbuf->f_mntfromname, "\\Device\\HarddiskVolume", 22);
	for (int i = 22; u16_ntpath->Buffer[i] != L'\0' && u16_ntpath->Buffer[i] != L'\\'; ++i)
	{
		statfsbuf->f_mntfromname[i] = (char)u16_ntpath->Buffer[i];
	}

	u16_dospath = ntpath_to_dospath(u16_ntpath);
	RtlFreeHeap(NtCurrentProcessHeap(), 0, u16_ntpath);

	if (u16_dospath == NULL)
	{
		// errno will be set by `ntpath_to_dospath`.
		return -1;
	}

	statfsbuf->f_mntonname[0] = (char)u16_dospath->Buffer[0];
	statfsbuf->f_mntonname[1] = ':';

	RtlFreeHeap(NtCurrentProcessHeap(), 0, u16_dospath);

	// TODO removable
	return 0;
}

int wlibc_common_statfs(int fd, const char *restrict path, struct statfs *restrict statfsbuf)
{
	if (statfsbuf == NULL)
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

		return do_statfs(info.handle, statfsbuf);
	}
	else
	{
		int result;
		HANDLE handle;

		handle = just_open(AT_FDCWD, path, FILE_READ_ATTRIBUTES, 0);
		if (handle == INVALID_HANDLE_VALUE)
		{
			// errno will be set by `just_open`
			return -1;
		}

		result = do_statfs(handle, statfsbuf);
		NtClose(handle);

		return result;
	}
}
