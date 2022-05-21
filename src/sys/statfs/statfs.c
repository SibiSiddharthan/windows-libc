/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <internal/fcntl.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/statfs.h>

int do_statfs(HANDLE handle, struct statfs *restrict statfsbuf)
{
	NTSTATUS status;
	IO_STATUS_BLOCK io;
	FILE_FS_FULL_SIZE_INFORMATION fs_info;
	FILE_FS_VOLUME_INFORMATION *volume_info;
	char volume_info_buffer[128];

	// In Windows there are no inodes. Ignore these fields.
	statfsbuf->f_files = -1ull;
	statfsbuf->f_ffree = -1ull;
	statfsbuf->f_favail = -1ull;

	// Only these flags are valid here.
	statfsbuf->f_flag = MNT_NOSUID;

	// Max path length on Windows.
	statfsbuf->f_namemax = 32768;

	status = NtQueryVolumeInformationFile(handle, &io, &fs_info, sizeof(FILE_FS_FULL_SIZE_INFORMATION), FileFsFullSizeInformation);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	statfsbuf->f_bsize = fs_info.BytesPerSector * fs_info.SectorsPerAllocationUnit;
	statfsbuf->f_iosize = statfsbuf->f_bsize; // Treate these two fields as the same.
	statfsbuf->f_blocks = fs_info.TotalAllocationUnits.QuadPart;
	statfsbuf->f_bfree = fs_info.ActualAvailableAllocationUnits.QuadPart;
	statfsbuf->f_bavail = fs_info.CallerAvailableAllocationUnits.QuadPart;

	volume_info = (PFILE_FS_VOLUME_INFORMATION)volume_info_buffer;
	status = NtQueryVolumeInformationFile(handle, &io, volume_info, 128, FileFsVolumeInformation);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	statfsbuf->f_fsid.major = volume_info->VolumeSerialNumber;
	statfsbuf->f_fsid.minor = 0;

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
