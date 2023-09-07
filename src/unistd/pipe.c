/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <internal/fcntl.h>
#include <internal/path.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

static unsigned long roundup_16384(size_t size)
{
	return (((unsigned long)size + 16383) / 16384) * 16384;
}

int create_anonymous_pipe(HANDLE *read_handle, HANDLE *write_handle, int flags, size_t size)
{
	NTSTATUS status;
	IO_STATUS_BLOCK io;
	OBJECT_ATTRIBUTES object;
	UNICODE_STRING root_pipe, empty = {0, 0, NULL};
	HANDLE root_handle;
	ULONG share = FILE_SHARE_READ | FILE_SHARE_WRITE;
	ULONG attributes = OBJ_CASE_INSENSITIVE | OBJ_INHERIT;
	ULONG options = FILE_SYNCHRONOUS_IO_NONALERT;
	ULONG buffer_size = roundup_16384(size);
	LARGE_INTEGER timeout;

	// We don't support O_DIRECT for pipes.
	if (flags & O_CLOEXEC)
	{
		attributes &= ~OBJ_INHERIT;
	}
	if (flags & O_NONBLOCK)
	{
		attributes &= ~FILE_SYNCHRONOUS_IO_NONALERT;
	}

	// The `CreatePipe` call uses 120 seconds as the default timeout value. Let's stick with that.
	timeout.QuadPart = -1200000000;

	// Path of root pipe.
	RtlInitUnicodeString(&root_pipe, L"\\Device\\NamedPipe\\");
	InitializeObjectAttributes(&object, &root_pipe, OBJ_CASE_INSENSITIVE, NULL, NULL);

	status = NtCreateFile(&root_handle, SYNCHRONIZE, &object, &io, NULL, 0, share, FILE_OPEN, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	InitializeObjectAttributes(&object, &empty, attributes, root_handle, NULL);
	// NOTE: This call will always fail if timeout is not given.
	// This will create an unnamed pipe believe me.
	status = NtCreateNamedPipeFile(read_handle, FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES | FILE_READ_DATA | SYNCHRONIZE, &object, &io,
								   share, FILE_CREATE, options, FILE_PIPE_BYTE_STREAM_TYPE, FILE_PIPE_BYTE_STREAM_MODE,
								   (flags & O_NONBLOCK) == 0 ? FILE_PIPE_SYNCHRONOUS_OPERATION : FILE_PIPE_NONBLOCKING_OPERATION, 1,
								   buffer_size, buffer_size, &timeout);

	// We don't need this anymore, close it.
	NtClose(root_handle);

	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	// To open the other end of the pipe, use the handle of the pipe we created as its root.
	InitializeObjectAttributes(&object, &empty, attributes, *read_handle, NULL);
	status =
		NtCreateFile(write_handle, FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES | FILE_WRITE_DATA | FILE_CREATE_PIPE_INSTANCE | SYNCHRONIZE,
					 &object, &io, NULL, 0, share, FILE_OPEN, options, NULL, 0);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		NtClose(*read_handle);
		return -1;
	}

	return 0;
}

int create_named_pipe(UNICODE_STRING *path, HANDLE *read_handle, HANDLE *write_handle, int flags, size_t size)
{
	NTSTATUS status;
	IO_STATUS_BLOCK io;
	OBJECT_ATTRIBUTES object;
	UNICODE_STRING empty = {0, 0, NULL};
	ULONG share = FILE_SHARE_READ | FILE_SHARE_WRITE;
	ULONG attributes = OBJ_CASE_INSENSITIVE | OBJ_INHERIT;
	ULONG options = FILE_SYNCHRONOUS_IO_NONALERT;
	ULONG buffer_size = roundup_16384(size);
	LARGE_INTEGER timeout;

	// We don't support O_DIRECT for pipes.
	if (flags & O_CLOEXEC)
	{
		attributes &= ~OBJ_INHERIT;
	}
	if (flags & O_NONBLOCK)
	{
		attributes &= ~FILE_SYNCHRONOUS_IO_NONALERT;
	}

	// The `CreatePipe` call uses 120 seconds as the default timeout value. Let's stick with that.
	timeout.QuadPart = -1200000000;

	// Path of root pipe.
	InitializeObjectAttributes(&object, path, OBJ_CASE_INSENSITIVE, NULL, NULL);
	status = NtCreateNamedPipeFile(read_handle, FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES | FILE_READ_DATA | SYNCHRONIZE, &object, &io,
								   share, FILE_CREATE, options, FILE_PIPE_BYTE_STREAM_TYPE, FILE_PIPE_BYTE_STREAM_MODE,
								   (flags & O_NONBLOCK) == 0 ? FILE_PIPE_SYNCHRONOUS_OPERATION : FILE_PIPE_NONBLOCKING_OPERATION, 1,
								   buffer_size, buffer_size, &timeout);

	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}

	// To open the other end of the pipe, use the handle of the pipe we created as its root.
	InitializeObjectAttributes(&object, &empty, attributes, *read_handle, NULL);
	status =
		NtCreateFile(write_handle, FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES | FILE_WRITE_DATA | FILE_CREATE_PIPE_INSTANCE | SYNCHRONIZE,
					 &object, &io, NULL, 0, share, FILE_OPEN, options, NULL, 0);

	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		NtClose(*read_handle);
		return -1;
	}

	return 0;
}

int wlibc_common_pipe(const char *name, int pipefd[2], int flags, size_t size)
{
	int status;
	int read_fd, write_fd;
	handle_t type = INVALID_HANDLE;
	HANDLE read_handle, write_handle;
	UNICODE_STRING *path = NULL;

	if ((flags & ~(O_CLOEXEC | O_DIRECT | O_NONBLOCK)) != 0)
	{
		errno = EINVAL;
		return -1;
	}

	if (name != NULL)
	{
		path = get_absolute_ntpath2(AT_FDCWD, name, &type);
		if (type != PIPE_HANDLE)
		{
			goto fail;
		}

		status = create_named_pipe(path, &read_handle, &write_handle, flags, size);
	}
	else
	{
		status = create_anonymous_pipe(&read_handle, &write_handle, flags, size);
	}

	if (status != 0)
	{
		// Any opened handles will be closed and errno will be set.
		goto fail;
	}

	read_fd = register_to_fd_table(read_handle, PIPE_HANDLE, O_RDONLY | flags);
	write_fd = register_to_fd_table(write_handle, PIPE_HANDLE, O_WRONLY | flags);

	pipefd[0] = read_fd;
	pipefd[1] = write_fd;

	return 0;

fail:
	if (path != NULL)
	{
		RtlFreeHeap(NtCurrentProcessHeap(), 0, path);
	}

	return -1;
}
