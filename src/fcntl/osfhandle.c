/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <internal/fcntl.h>
#include <fcntl.h>

intptr_t wlibc_get_osfhandle(int fd)
{
	if (!validate_fd(fd))
	{
		errno = EBADF;
		return (intptr_t)INVALID_HANDLE_VALUE;
	}

	return (intptr_t)get_fd_handle(fd);
}

int wlibc_open_osfhandle(intptr_t handle, int flags)
{
	if (handle == (intptr_t)INVALID_HANDLE_VALUE || handle == (intptr_t)NULL)
	{
		errno = EINVAL;
		return -1;
	}

	NTSTATUS status;
	IO_STATUS_BLOCK io;
	FILE_FS_DEVICE_INFORMATION device_info;
	FILE_ATTRIBUTE_TAG_INFORMATION attribute_info;
	int fd;
	handle_t htype;

	status = NtQueryVolumeInformationFile((HANDLE)handle, &io, &device_info, sizeof(FILE_FS_DEVICE_INFORMATION), FileFsDeviceInformation);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	if (device_info.DeviceType == FILE_DEVICE_DISK)
	{
		status = NtQueryInformationFile((HANDLE)handle, &io, &attribute_info, sizeof(FILE_ATTRIBUTE_TAG_INFORMATION),
										FileAttributeTagInformation);
		if (status != STATUS_SUCCESS)
		{
			map_ntstatus_to_errno(status);
			return -1;
		}

		if (attribute_info.FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			htype = DIRECTORY_HANDLE;
		}
		else
		{
			htype = FILE_HANDLE;
		}
	}
	else if (device_info.DeviceType == FILE_DEVICE_NAMED_PIPE)
	{
		htype = PIPE_HANDLE;
	}
	else if (device_info.DeviceType == FILE_DEVICE_CONSOLE)
	{
		htype = CONSOLE_HANDLE;
	}
	else if (device_info.DeviceType == FILE_DEVICE_NULL)
	{
		htype = NULL_HANDLE;
	}
	else
	{
		// Error
		htype = INVALID_HANDLE;
		return -1;
	}

	fd = register_to_fd_table((HANDLE)handle, htype, flags);
	return fd;
}
