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
#include <stdlib.h>

struct fd_table *_wlibc_fd_table = NULL;
size_t _wlibc_fd_table_size = 0;
unsigned int _wlibc_fd_sequence = 0;
RTL_SRWLOCK _wlibc_fd_table_srwlock;

// Declaration of static functions
static int internal_insert_fd(int index, HANDLE _h, handle_t _type, int _flags);
static int register_to_fd_table_internal(HANDLE _h, handle_t _type, int _flags);
static void insert_into_fd_table_internal(int _fd, HANDLE _h, handle_t _type, int _flags);
static void unregister_from_fd_table_internal(HANDLE _h);
static int get_fd_internal(HANDLE _h);
static int close_fd_internal(int _fd);

static HANDLE get_fd_handle_internal(int _fd);
static int get_fd_flags_internal(int _fd);
static handle_t get_fd_type_internal(int _fd);

static void set_fd_handle_internal(int _fd, HANDLE _handle);
static void set_fd_flags_internal(int _fd, int _flags);
static void set_fd_type_internal(int _fd, handle_t _type);
static void add_fd_flags_internal(int _fd, int _flags);

static bool validate_fd_internal(int _fd);

handle_t determine_type(DEVICE_TYPE type)
{
	switch (type)
	{
	case FILE_DEVICE_NULL:
		return NULL_HANDLE;
	case FILE_DEVICE_CONSOLE:
		return CONSOLE_HANDLE;
	case FILE_DEVICE_DISK: // Assume no one is going to give a directory handle as a std stream
		return FILE_HANDLE;
	case FILE_DEVICE_NAMED_PIPE:
		return PIPE_HANDLE;
	default:
		return INVALID_HANDLE;
	}
}

// Opening the console handle should not fail as we are only doing it if there is a console attached.
HANDLE open_conin(void)
{
	HANDLE handle = INVALID_HANDLE_VALUE;
	IO_STATUS_BLOCK io;
	UNICODE_STRING name;
	OBJECT_ATTRIBUTES object;

	name.Buffer = L"\\Device\\ConDrv\\CurrentIn";
	name.Length = 48;
	name.MaximumLength = 50;

	InitializeObjectAttributes(&object, &name, OBJ_CASE_INSENSITIVE | OBJ_INHERIT, NULL, NULL);
	NtCreateFile(&handle, FILE_GENERIC_READ, &object, &io, NULL, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, FILE_OPEN,
				 FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
	return handle;
}

HANDLE open_conout(void)
{
	HANDLE handle = INVALID_HANDLE_VALUE;
	IO_STATUS_BLOCK io;
	UNICODE_STRING name;
	OBJECT_ATTRIBUTES object;

	name.Buffer = L"\\Device\\ConDrv\\CurrentOut";
	name.Length = 50;
	name.MaximumLength = 52;

	InitializeObjectAttributes(&object, &name, OBJ_CASE_INSENSITIVE | OBJ_INHERIT, NULL, NULL);
	NtCreateFile(&handle, FILE_GENERIC_WRITE, &object, &io, NULL, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, FILE_OPEN,
				 FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
	return handle;
}

///////////////////////////////////////
// Initialization and cleanup functions
///////////////////////////////////////
void init_fd_table(void)
{
	RtlInitializeSRWLock(&_wlibc_fd_table_srwlock);
	bool console_subsystem = true;
	if (NtCurrentPeb()->ProcessParameters->ConsoleHandle == 0 || NtCurrentPeb()->ProcessParameters->ConsoleHandle == (HANDLE)-1)
	{
		console_subsystem = false;
	}
	HANDLE hin = NtCurrentPeb()->ProcessParameters->StandardInput;
	HANDLE hout = NtCurrentPeb()->ProcessParameters->StandardOutput;
	HANDLE herr = NtCurrentPeb()->ProcessParameters->StandardError;

	NTSTATUS status;
	IO_STATUS_BLOCK io;
	FILE_FS_DEVICE_INFORMATION device_info;

	_wlibc_fd_table_size = 4;
	_wlibc_fd_table = (struct fd_table *)malloc(sizeof(struct fd_table) * _wlibc_fd_table_size);

	status = NtQueryVolumeInformationFile(hin, &io, &device_info, sizeof(FILE_FS_DEVICE_INFORMATION), FileFsDeviceInformation);
	if (status == STATUS_SUCCESS)
	{
		_wlibc_fd_table[0].handle = hin;
		_wlibc_fd_table[0].type = determine_type(device_info.DeviceType);
		_wlibc_fd_table[0].flags = O_RDONLY;
	}
	else if (console_subsystem)
	{
		_wlibc_fd_table[0].handle = open_conin();
		if (_wlibc_fd_table[0].handle != INVALID_HANDLE_VALUE)
		{
			_wlibc_fd_table[0].type = CONSOLE_HANDLE;
			_wlibc_fd_table[0].flags = O_RDONLY;
			_wlibc_fd_table[0].sequence = 1;
		}
		else
		{
			_wlibc_fd_table[0].handle = INVALID_HANDLE_VALUE;
		}
	}

	status = NtQueryVolumeInformationFile(hout, &io, &device_info, sizeof(FILE_FS_DEVICE_INFORMATION), FileFsDeviceInformation);
	if (status == STATUS_SUCCESS)
	{
		_wlibc_fd_table[1].handle = hout;
		_wlibc_fd_table[1].type = determine_type(device_info.DeviceType);
		_wlibc_fd_table[1].flags = O_WRONLY | (device_info.DeviceType == FILE_DEVICE_NAMED_PIPE ? O_APPEND : 0);
	}
	else if (console_subsystem)
	{
		_wlibc_fd_table[1].handle = open_conout();
		if (_wlibc_fd_table[1].handle != INVALID_HANDLE_VALUE)
		{
			_wlibc_fd_table[1].type = CONSOLE_HANDLE;
			_wlibc_fd_table[1].flags = O_RDONLY;
		}
		else
		{
			_wlibc_fd_table[1].handle = INVALID_HANDLE_VALUE;
		}
	}

	status = NtQueryVolumeInformationFile(herr, &io, &device_info, sizeof(FILE_FS_DEVICE_INFORMATION), FileFsDeviceInformation);
	if (status == STATUS_SUCCESS)
	{
		_wlibc_fd_table[2].handle = herr;
		_wlibc_fd_table[2].type = determine_type(device_info.DeviceType);
		_wlibc_fd_table[2].flags = O_WRONLY | (device_info.DeviceType == FILE_DEVICE_NAMED_PIPE ? O_APPEND : 0);
	}
	else if (console_subsystem)
	{
		_wlibc_fd_table[2].handle = open_conout();
		if (_wlibc_fd_table[2].handle != INVALID_HANDLE_VALUE)
		{
			_wlibc_fd_table[2].type = CONSOLE_HANDLE;
			_wlibc_fd_table[2].flags = O_RDONLY;
		}
		else
		{
			_wlibc_fd_table[2].handle = INVALID_HANDLE_VALUE;
		}
	}

	// Cygwin/MSYS gives the same handle as stdout and stderr, duplicate the handle
	if ((_wlibc_fd_table[1].handle != INVALID_HANDLE_VALUE && _wlibc_fd_table[2].handle != INVALID_HANDLE_VALUE) &&
		_wlibc_fd_table[1].handle == _wlibc_fd_table[2].handle)
	{
		HANDLE new_stderr;
		status = NtDuplicateObject(NtCurrentProcess(), _wlibc_fd_table[1].handle, NtCurrentProcess(), &new_stderr, 0, 0,
								   DUPLICATE_SAME_ACCESS | DUPLICATE_SAME_ATTRIBUTES);
		if (status == STATUS_SUCCESS)
		{
			// Don't do anything in case of failure
			_wlibc_fd_table[2].handle = new_stderr;
		}
	}

	// Initialize the sequence numbers for the std handles.
	// Don't worry if the handles are uninitialized.
	_wlibc_fd_table[0].sequence = ++_wlibc_fd_sequence;
	_wlibc_fd_table[1].sequence = ++_wlibc_fd_sequence;
	_wlibc_fd_table[2].sequence = ++_wlibc_fd_sequence;

	// set the remaining file descriptors as free, since we are allocating in powers of 2
	_wlibc_fd_table[3].handle = INVALID_HANDLE_VALUE;
}

// Not worrying about open handles (not closed by the user)
void cleanup_fd_table(void)
{
	free(_wlibc_fd_table);
}

///////////////////////////////////////
// Primary functions
///////////////////////////////////////

static int internal_insert_fd(int index, HANDLE _h, handle_t _type, int _flags)
{
	_wlibc_fd_table[index].handle = _h;
	_wlibc_fd_table[index].flags = _flags;
	_wlibc_fd_table[index].type = _type;
	_wlibc_fd_table[index].sequence = ++_wlibc_fd_sequence;
	return index;
}

static int register_to_fd_table_internal(HANDLE _h, handle_t _type, int _flags)
{
	bool is_there_free_space = false;
	int index = -1;
	int fd = -1;
	for (size_t i = 0; i < _wlibc_fd_table_size; i++)
	{
		if (_wlibc_fd_table[i].handle == INVALID_HANDLE_VALUE)
		{
			is_there_free_space = true;
			index = (int)i;
			break;
		}
	}

	if (is_there_free_space)
	{
		fd = internal_insert_fd(index, _h, _type, _flags);
	}

	else // double the table size
	{
		struct fd_table *temp = (struct fd_table *)malloc(sizeof(struct fd_table) * _wlibc_fd_table_size * 2);
		memcpy(temp, _wlibc_fd_table, sizeof(struct fd_table) * _wlibc_fd_table_size);
		free(_wlibc_fd_table);
		_wlibc_fd_table = temp;

		fd = internal_insert_fd((int)_wlibc_fd_table_size, _h, _type, _flags);

		for (size_t i = _wlibc_fd_table_size + 1; i < _wlibc_fd_table_size * 2; i++)
		{
			_wlibc_fd_table[i].handle = INVALID_HANDLE_VALUE;
		}

		_wlibc_fd_table_size *= 2;
	}

	return fd;
}

int register_to_fd_table(HANDLE _h, handle_t _type, int _flags)
{
	EXCLUSIVE_LOCK_FD_TABLE();
	int fd = register_to_fd_table_internal(_h, _type, _flags);
	EXCLUSIVE_UNLOCK_FD_TABLE();
	return fd;
}

static void insert_into_fd_table_internal(int _fd, HANDLE _h, handle_t _type, int _flags)
{
	// grow the table
	if (_fd >= (int)_wlibc_fd_table_size)
	{
		struct fd_table *temp = (struct fd_table *)malloc(sizeof(struct fd_table) * _fd * 2); // Allocate double the requested fd number
		memcpy(temp, _wlibc_fd_table, sizeof(struct fd_table) * _wlibc_fd_table_size);
		free(_wlibc_fd_table);
		_wlibc_fd_table = temp;

		for (int i = (int)_wlibc_fd_table_size; i < _fd * 2; ++i)
		{
			_wlibc_fd_table[i].handle = INVALID_HANDLE_VALUE;
		}

		_wlibc_fd_table_size = _fd * 2;
	}

	internal_insert_fd(_fd, _h, _type, _flags);
}

void insert_into_fd_table(int _fd, HANDLE _h, handle_t _type, int _flags)
{
	EXCLUSIVE_LOCK_FD_TABLE();
	insert_into_fd_table_internal(_fd, _h, _type, _flags);
	EXCLUSIVE_UNLOCK_FD_TABLE();
}

static void unregister_from_fd_table_internal(HANDLE _h)
{
	for (size_t i = 0; i < _wlibc_fd_table_size; i++)
	{
		if (_wlibc_fd_table[i].handle != INVALID_HANDLE_VALUE && _wlibc_fd_table[i].handle == _h)
		{
			// Calling this function after the handle has been closed
			_wlibc_fd_table[i].handle = INVALID_HANDLE_VALUE;
			break;
		}
	}
}

void unregister_from_fd_table(HANDLE _h)
{
	EXCLUSIVE_LOCK_FD_TABLE();
	unregister_from_fd_table_internal(_h);
	EXCLUSIVE_UNLOCK_FD_TABLE();
}

static int get_fd_internal(HANDLE _h)
{
	for (size_t i = 0; i < _wlibc_fd_table_size; i++)
	{
		if (_wlibc_fd_table[i].handle != INVALID_HANDLE_VALUE && _h == _wlibc_fd_table[i].handle)
		{
			return (int)i;
		}
	}

	errno = EBADF;
	return -1;
}

int get_fd(HANDLE _h)
{
	SHARED_LOCK_FD_TABLE();
	int fd = get_fd_internal(_h);
	SHARED_UNLOCK_FD_TABLE();
	return fd;
}

static int close_fd_internal(int _fd)
{
	NTSTATUS status = NtClose(_wlibc_fd_table[_fd].handle);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}
	// Closing the file descriptor. Mark the handle as invalid, so we can reuse the same fd again.
	_wlibc_fd_table[_fd].handle = INVALID_HANDLE_VALUE;
	return 0;
}

int close_fd(int _fd)
{
	EXCLUSIVE_LOCK_FD_TABLE();
	int status = close_fd_internal(_fd);
	EXCLUSIVE_UNLOCK_FD_TABLE();
	return status;
}

///////////////////////////////////////
// Getters
///////////////////////////////////////
static HANDLE get_fd_handle_internal(int _fd)
{
	return _wlibc_fd_table[_fd].handle;
}

HANDLE get_fd_handle(int _fd)
{
	HANDLE handle;
	SHARED_LOCK_FD_TABLE();
	handle = get_fd_handle_internal(_fd);
	SHARED_UNLOCK_FD_TABLE();
	return handle;
}

static int get_fd_flags_internal(int _fd)
{
	return _wlibc_fd_table[_fd].flags;
}

int get_fd_flags(int _fd)
{
	int flags;
	SHARED_LOCK_FD_TABLE();
	flags = get_fd_flags_internal(_fd);
	SHARED_UNLOCK_FD_TABLE();
	return flags;
}

static handle_t get_fd_type_internal(int _fd)
{
	if (validate_fd_internal(_fd))
		return _wlibc_fd_table[_fd].type;
	else
		return INVALID_HANDLE;
}

handle_t get_fd_type(int _fd)
{
	handle_t type;
	SHARED_LOCK_FD_TABLE();
	type = get_fd_type_internal(_fd);
	SHARED_UNLOCK_FD_TABLE();
	return type;
}

///////////////////////////////////////
// Setters
///////////////////////////////////////
static void set_fd_handle_internal(int _fd, HANDLE _handle)
{
	_wlibc_fd_table[_fd].handle = _handle;
}

void set_fd_handle(int _fd, HANDLE _handle)
{
	EXCLUSIVE_LOCK_FD_TABLE();
	set_fd_handle_internal(_fd, _handle);
	EXCLUSIVE_UNLOCK_FD_TABLE();
}

static void set_fd_flags_internal(int _fd, int _flags)
{
	_wlibc_fd_table[_fd].flags = _flags;
}

void set_fd_flags(int _fd, int _flags)
{
	EXCLUSIVE_LOCK_FD_TABLE();
	set_fd_flags_internal(_fd, _flags);
	EXCLUSIVE_UNLOCK_FD_TABLE();
}

static void set_fd_type_internal(int _fd, handle_t _type)
{
	_wlibc_fd_table[_fd].type = _type;
}

void set_fd_type(int _fd, handle_t _type)
{
	EXCLUSIVE_LOCK_FD_TABLE();
	set_fd_type_internal(_fd, _type);
	EXCLUSIVE_UNLOCK_FD_TABLE();
}

static void add_fd_flags_internal(int _fd, int _flags)
{
	_wlibc_fd_table[_fd].flags |= _flags;
}

void add_fd_flags(int _fd, int _flags)
{
	EXCLUSIVE_LOCK_FD_TABLE();
	add_fd_flags_internal(_fd, _flags);
	EXCLUSIVE_UNLOCK_FD_TABLE();
}

///////////////////////////////////////
// Validators
///////////////////////////////////////
static bool validate_fd_internal(int _fd)
{
	if (_fd < 0 || _fd >= (int)_wlibc_fd_table_size)
		return false;
	if (_wlibc_fd_table[_fd].handle == INVALID_HANDLE_VALUE)
		return false;
	return true;
}

bool validate_fd(int _fd)
{
	bool condition = false;
	SHARED_LOCK_FD_TABLE();
	condition = validate_fd_internal(_fd);
	SHARED_UNLOCK_FD_TABLE();
	return condition;
}
