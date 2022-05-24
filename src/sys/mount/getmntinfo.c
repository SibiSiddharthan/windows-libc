/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <internal/fcntl.h>
#include <errno.h>
#include <alloca.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/param.h>
#include <sys/statfs.h>

int do_statfs(HANDLE handle, struct statfs *restrict statfsbuf);

int wlibc_getmntinfo(struct statfs **mounts, int mode)
{
	NTSTATUS status;
	IO_STATUS_BLOCK io;
	HANDLE mountmgr_handle;
	MOUNTMGR_MOUNT_POINT mount = {0};
	MOUNTMGR_MOUNT_POINTS *mount_points = NULL;
	VOID *mount_points_buffer = NULL;
	DWORD drive_count = 0;
	ULONG required_size = 4096; // Start with 4096 bytes

	UNREFERENCED_PARAMETER(mode);

	if (mounts == NULL)
	{
		errno = EINVAL;
		return -1;
	}

	// This should not fail, but just in case.
	mountmgr_handle = open_mountmgr();
	if (mountmgr_handle == INVALID_HANDLE_VALUE)
	{
		return -1;
	}

	mount_points_buffer = malloc(required_size);
	memset(mount_points_buffer, 0, required_size);
	mount_points = (PMOUNTMGR_MOUNT_POINTS)mount_points_buffer;

	status = NtDeviceIoControlFile(mountmgr_handle, NULL, NULL, NULL, &io, IOCTL_MOUNTMGR_QUERY_POINTS, &mount,
								   sizeof(MOUNTMGR_MOUNT_POINT), mount_points, required_size);
	if (status != STATUS_SUCCESS)
	{
		if (status != STATUS_BUFFER_OVERFLOW)
		{
			required_size = roundup(mount_points->Size, 4096);
			VOID *temp = mount_points_buffer;

			mount_points_buffer = realloc(mount_points_buffer, required_size);
			memset(mount_points_buffer, 0, required_size);
			mount_points = (PMOUNTMGR_MOUNT_POINTS)mount_points_buffer;
			free(temp);

			status = NtDeviceIoControlFile(mountmgr_handle, NULL, NULL, NULL, &io, IOCTL_MOUNTMGR_QUERY_POINTS, &mount,
										   sizeof(MOUNTMGR_MOUNT_POINT), mount_points, required_size);
			if (status != STATUS_SUCCESS)
			{
				map_ntstatus_to_errno(status);
				goto finish;
			}
		}
		else
		{
			map_ntstatus_to_errno(status);
			goto finish;
		}
	}

	CHAR *drives = alloca(mount_points->NumberOfMountPoints);

	for (ULONG i = 0; i < mount_points->NumberOfMountPoints; ++i)
	{
		// There will be two kinds of mount points.
		// One kind maps them to logical drives like C:, D: etc. These start with "\DosDevices".
		// The other kind start with "\??\Volume{GUID}". I don't know what these are. Ignore them for now.
		if (memcmp((CHAR *)mount_points + mount_points->MountPoints[i].SymbolicLinkNameOffset, L"\\DosDevices\\", 12 * sizeof(WCHAR)) == 0)
		{
			drives[i] = 1;
			++drive_count;
		}
		else
		{
			drives[i] = 0;
		}
	}

	*mounts = (struct statfs *)malloc(sizeof(struct statfs) * drive_count);
	DWORD index = 0;

	for (ULONG i = 0; i < mount_points->NumberOfMountPoints; ++i)
	{
		UNICODE_STRING drive_path;
		HANDLE drive_handle;

		if (drives[i])
		{
			// The mount points are listed in the buffer one after the another.
			// The device name is always listed before the symbolic names.
			// Say we have C:, D: drives. The listing will look like this.
			// "*\Device\HarddiskVolume1\DosDevices\A\Device\HarddiskVolume2\DosDevices\D*"
			// So we will always have a trailing slash, which we need for `statfs`.
			drive_path.Buffer = (WCHAR *)((CHAR *)mount_points + mount_points->MountPoints[i].DeviceNameOffset);
			drive_path.Length = mount_points->MountPoints[i].DeviceNameLength + sizeof(WCHAR);
			drive_path.MaximumLength = drive_path.Length;

			// Ignore errors here.
			drive_handle = just_open2(&drive_path, FILE_READ_ATTRIBUTES, 0);
			if (drive_handle != INVALID_HANDLE_VALUE)
			{
				do_statfs(drive_handle, (*mounts + index++));
				NtClose(drive_handle);
			}
		}
	}

finish:
	NtClose(mountmgr_handle);
	free(mount_points_buffer);
	return drive_count;
}
