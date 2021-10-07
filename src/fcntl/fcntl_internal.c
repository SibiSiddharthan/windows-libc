/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/fcntl.h>
#include <Windows.h>
#include <stdlib.h>
#include <internal/error.h>
#include <internal/nt.h>
#include <errno.h>
#include <fcntl.h>

struct fd_table *_fd_io = NULL;
size_t _fd_table_size = 0;
CRITICAL_SECTION _fd_critical;

// Declaration of static functions
static int internal_insert_fd(int index, HANDLE _h, const wchar_t *_path, enum handle_type _type, int _flags);
static int register_to_fd_table_internal(HANDLE _h, const wchar_t *_path, enum handle_type _type, int _flags);
static void update_fd_table_internal(int _fd, HANDLE _h, const wchar_t *_path, enum handle_type _type, int _flags);
static void insert_into_fd_table_internal(int _fd, HANDLE _h, const wchar_t *_path, enum handle_type _type, int _flags);
static void unregister_from_fd_table_internal(HANDLE _h);
static int get_fd_internal(HANDLE _h);
static int close_fd_internal(int _fd);

static HANDLE get_fd_handle_internal(int _fd);
static int get_fd_flags_internal(int _fd);
static enum handle_type get_fd_type_internal(int _fd);
static const wchar_t *get_path_internal(int _fd);

static void set_fd_handle_internal(int _fd, HANDLE _handle);
static void set_fd_flags_internal(int _fd, int _flags);
static void set_fd_type_internal(int _fd, enum handle_type _type);
static void set_fd_path_internal(int _fd, const wchar_t *_path);
static void add_fd_flags_internal(int _fd, int _flags);

static bool validate_fd_internal(int _fd);

enum handle_type determine_type(DEVICE_TYPE type)
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

HANDLE open_conin()
{
	HANDLE handle = INVALID_HANDLE_VALUE;
	IO_STATUS_BLOCK io;
	UNICODE_STRING name;
	OBJECT_ATTRIBUTES object;
	RtlInitUnicodeString(&name, L"\\??\\CON");
	InitializeObjectAttributes(&object, &name, OBJ_CASE_INSENSITIVE | OBJ_INHERIT, NULL, NULL);
	NTSTATUS status = NtCreateFile(&handle, FILE_GENERIC_READ, &object, &io, NULL, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, FILE_OPEN,
								   FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
	return handle;
}

HANDLE open_conout()
{
	HANDLE handle = INVALID_HANDLE_VALUE;
	IO_STATUS_BLOCK io;
	UNICODE_STRING name;
	OBJECT_ATTRIBUTES object;
	RtlInitUnicodeString(&name, L"\\??\\CON");
	InitializeObjectAttributes(&object, &name, OBJ_CASE_INSENSITIVE | OBJ_INHERIT, NULL, NULL);
	NTSTATUS status = NtCreateFile(&handle, FILE_GENERIC_WRITE, &object, &io, NULL, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, FILE_OPEN,
								   FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
	return handle;
}

///////////////////////////////////////
// Initialization and cleanup functions
///////////////////////////////////////
void init_fd_table()
{
	InitializeCriticalSection(&_fd_critical);
	// HANDLE hin = GetStdHandle(STD_INPUT_HANDLE);
	// HANDLE hout = GetStdHandle(STD_OUTPUT_HANDLE);
	// HANDLE herr = GetStdHandle(STD_ERROR_HANDLE);
	bool console_subsystem = true;
	if (NtCurrentPeb()->ProcessParameters->ConsoleHandle == 0 || NtCurrentPeb()->ProcessParameters->ConsoleHandle == (HANDLE)-1)
	{
		console_subsystem = false;
	}
	HANDLE hin = NtCurrentPeb()->ProcessParameters->StandardInput;
	HANDLE hout = NtCurrentPeb()->ProcessParameters->StandardOutput;
	HANDLE herr = NtCurrentPeb()->ProcessParameters->StandardError;

	IO_STATUS_BLOCK I;
	NTSTATUS status;
	FILE_FS_DEVICE_INFORMATION device_info;

	_fd_io = (struct fd_table *)malloc(sizeof(struct fd_table) * 4);
	_fd_table_size = 4;

	status = NtQueryVolumeInformationFile(hin, &I, &device_info, sizeof(FILE_FS_DEVICE_INFORMATION), FileFsDeviceInformation);
	if (status == STATUS_SUCCESS)
	{
		_fd_io[0]._handle = hin;
		_fd_io[0]._type = determine_type(device_info.DeviceType);
		_fd_io[0]._flags = O_RDONLY;
		_fd_io[0]._free = 0;
		wcscpy(_fd_io[0]._path, L"CON");
	}
	else if (console_subsystem)
	{
		_fd_io[0]._handle = open_conin();
		if (_fd_io[0]._handle != INVALID_HANDLE_VALUE)
		{
			_fd_io[0]._type = CONSOLE_HANDLE;
			_fd_io[0]._flags = O_RDONLY;
			_fd_io[0]._free = 0;
			wcscpy(_fd_io[0]._path, L"CON");
		}
		else
		{
			_fd_io[0]._free = 1;
		}
	}

	status = NtQueryVolumeInformationFile(hout, &I, &device_info, sizeof(FILE_FS_DEVICE_INFORMATION), FileFsDeviceInformation);
	if (status == STATUS_SUCCESS)
	{
		_fd_io[1]._handle = hout;
		_fd_io[1]._type = determine_type(device_info.DeviceType);
		_fd_io[1]._flags = O_WRONLY | (device_info.DeviceType == FILE_DEVICE_NAMED_PIPE ? O_APPEND : 0);
		_fd_io[1]._free = 0;
		wcscpy(_fd_io[1]._path, L"CON");
	}
	else if (console_subsystem)
	{
		_fd_io[1]._handle = open_conout();
		if (_fd_io[1]._handle != INVALID_HANDLE_VALUE)
		{
			_fd_io[1]._type = CONSOLE_HANDLE;
			_fd_io[1]._flags = O_RDONLY;
			_fd_io[1]._free = 0;
			wcscpy(_fd_io[1]._path, L"CON");
		}
		else
		{
			_fd_io[1]._free = 1;
		}
	}

	status = NtQueryVolumeInformationFile(herr, &I, &device_info, sizeof(FILE_FS_DEVICE_INFORMATION), FileFsDeviceInformation);
	if (status == STATUS_SUCCESS)
	{
		_fd_io[2]._handle = herr;
		_fd_io[2]._type = determine_type(device_info.DeviceType);
		_fd_io[2]._flags = O_WRONLY | (device_info.DeviceType == FILE_DEVICE_NAMED_PIPE ? O_APPEND : 0);
		_fd_io[2]._free = 0;
		wcscpy(_fd_io[2]._path, L"CON");
	}
	else if (console_subsystem)
	{
		_fd_io[2]._handle = open_conout();
		if (_fd_io[2]._handle != INVALID_HANDLE_VALUE)
		{
			_fd_io[2]._type = CONSOLE_HANDLE;
			_fd_io[2]._flags = O_RDONLY;
			_fd_io[2]._free = 0;
			wcscpy(_fd_io[2]._path, L"CON");
		}
		else
		{
			_fd_io[2]._free = 1;
		}
	}

	// Cygwin/MSYS gives the same handle as stdout and stderr, duplicate the handle
	if ((_fd_io[1]._free == 0 && _fd_io[2]._free == 0) && _fd_io[1]._handle == _fd_io[2]._handle)
	{
		HANDLE new_stderr;
		NTSTATUS status = NtDuplicateObject(NtCurrentProcess(), _fd_io[1]._handle, NtCurrentProcess(), &new_stderr, 0, 0,
											DUPLICATE_SAME_ACCESS | DUPLICATE_SAME_ATTRIBUTES);
		if (status == STATUS_SUCCESS)
		{
			// Don't do anything in case of failure
			_fd_io[2]._handle = new_stderr;
		}
	}

	_fd_io[3]._free = 1; // set the last one as free, since we are allocating in powers of 2
}

// Not worrying about open handles (not closed by the user)
void cleanup_fd_table()
{
	free(_fd_io);
	DeleteCriticalSection(&_fd_critical);
}

///////////////////////////////////////
// Primary functions
///////////////////////////////////////

static int internal_insert_fd(int index, HANDLE _h, const wchar_t *_path, enum handle_type _type, int _flags)
{
	_fd_io[index]._handle = _h;
	_fd_io[index]._flags = _flags;
	_fd_io[index]._type = _type;
	_fd_io[index]._free = 0;

	// Empty _path for pipes
	if (_type == PIPE_HANDLE)
	{
		_fd_io[index]._path[0] = L'\0';
		return index;
	}

	wcscpy(_fd_io[index]._path, _path);
	if (_type == DIRECTORY_HANDLE)
	{
		int length = wcslen(_path);
		// Append trailing slash
		if (_fd_io[index]._path[length - 1] != L'\\')
		{
			wcscat(_fd_io[index]._path, L"\\");
		}
	}

#if 0
	// We try to find the absolute path here
	// We need this for *at functions to work properly
	// This should not give an error
	wchar_t temp_buf[MAX_PATH];
	int length = GetFinalPathNameByHandle(_h, temp_buf, MAX_PATH, VOLUME_NAME_DOS);

	if (length == 0) // This will be true for a character device
	{
		wcscpy(_fd_io[index]._path, _path);
	}

	else
	{
		for (int i = 6; i < length; i++) // \\?\C:
		{
			if (temp_buf[i] == L'\\')
			{
				temp_buf[i] = L'/';
			}
		}

		wcscpy(_fd_io[index]._path, temp_buf + 4);

		// Append a trailing backslash(if necessary) if we are opening a directory
		if (_type == DIRECTORY_HANDLE)
		{
			length -= 4;
			if (!(_fd_io[index]._path[length - 1] == L'/'))
			{
				wcscat(_fd_io[index]._path, L"/");
			}
		}
	}
#endif
	return index;
}

static int register_to_fd_table_internal(HANDLE _h, const wchar_t *_path, enum handle_type _type, int _flags)
{
	bool is_there_free_space = 0;
	size_t index = -1;
	int fd = -1;
	for (size_t i = 0; i < _fd_table_size; i++)
	{
		if (_fd_io[i]._free)
		{
			is_there_free_space = 1;
			index = i;
			break;
		}
	}

	if (is_there_free_space)
	{
		fd = internal_insert_fd(index, _h, _path, _type, _flags);
	}

	else // double the table size
	{
		struct fd_table *temp = (struct fd_table *)malloc(sizeof(struct fd_table) * _fd_table_size);
		memcpy(temp, _fd_io, sizeof(struct fd_table) * _fd_table_size);
		_fd_io = (struct fd_table *)realloc(_fd_io, sizeof(struct fd_table) * _fd_table_size * 2);
		memcpy(_fd_io, temp, sizeof(struct fd_table) * (_fd_table_size));
		free(temp);

		fd = internal_insert_fd(_fd_table_size, _h, _path, _type, _flags);

		for (size_t i = _fd_table_size + 1; i < _fd_table_size * 2; i++)
		{
			_fd_io[i]._free = 1;
		}

		_fd_table_size *= 2;
	}

	return fd;
}

int register_to_fd_table(HANDLE _h, const wchar_t *_path, enum handle_type _type, int _flags)
{
	EnterCriticalSection(&_fd_critical);
	int fd = register_to_fd_table_internal(_h, _path, _type, _flags);
	LeaveCriticalSection(&_fd_critical);
	return fd;
}

static void update_fd_table_internal(int _fd, HANDLE _h, const wchar_t *_path, enum handle_type _type, int _flags)
{
	_fd_io[_fd]._handle = _h;
	_fd_io[_fd]._type = _type;
	_fd_io[_fd]._flags = _flags;
	set_fd_path_internal(_fd, _path);
}

void update_fd_table(int _fd, HANDLE _h, const wchar_t *_path, enum handle_type _type, int _flags)
{
	EnterCriticalSection(&_fd_critical);
	update_fd_table_internal(_fd, _h, _path, _type, _flags);
	LeaveCriticalSection(&_fd_critical);
}

static void insert_into_fd_table_internal(int _fd, HANDLE _h, const wchar_t *_path, enum handle_type _type, int _flags)
{
	// grow the table
	if (_fd >= _fd_table_size)
	{
		struct fd_table *temp = (struct fd_table *)malloc(sizeof(struct fd_table) * _fd_table_size);
		memcpy(temp, _fd_io, sizeof(struct fd_table) * _fd_table_size);
		_fd_io = (struct fd_table *)realloc(_fd_io, sizeof(struct fd_table) * _fd * 2); // Allocate double the requested fd number
		memcpy(_fd_io, temp, sizeof(struct fd_table) * (_fd_table_size));
		free(temp);

		for (size_t i = _fd_table_size; i < _fd * 2; i++)
		{
			_fd_io[i]._free = 1;
		}

		_fd_table_size = _fd * 2;
	}

	_fd_io[_fd]._free = 0;
	update_fd_table_internal(_fd, _h, _path, _type, _flags);
}

void insert_into_fd_table(int _fd, HANDLE _h, const wchar_t *_path, enum handle_type _type, int _flags)
{
	EnterCriticalSection(&_fd_critical);
	insert_into_fd_table_internal(_fd, _h, _path, _type, _flags);
	LeaveCriticalSection(&_fd_critical);
}

static void unregister_from_fd_table_internal(HANDLE _h)
{
	for (size_t i = 0; i < _fd_table_size; i++)
	{
		if (!_fd_io[i]._free && _fd_io[i]._handle == _h)
		{
			_fd_io[i]._free = 1;
			// Calling this function after the handle has been closed
			_fd_io[i]._handle = INVALID_HANDLE_VALUE;
			break;
		}
	}
}

void unregister_from_fd_table(HANDLE _h)
{
	EnterCriticalSection(&_fd_critical);
	unregister_from_fd_table_internal(_h);
	LeaveCriticalSection(&_fd_critical);
}

static int get_fd_internal(HANDLE _h)
{
	for (int i = 0; i < _fd_table_size; i++)
	{
		if (!_fd_io[i]._free && _h == _fd_io[i]._handle)
		{
			return i;
		}
	}

	errno = EBADF;
	return -1;
}

int get_fd(HANDLE _h)
{
	EnterCriticalSection(&_fd_critical);
	int fd = get_fd_internal(_h);
	LeaveCriticalSection(&_fd_critical);
	return fd;
}

static int close_fd_internal(int _fd)
{
	NTSTATUS status = NtClose(_fd_io[_fd]._handle);
	if (status != STATUS_SUCCESS)
	{
		map_ntstatus_to_errno(status);
		return -1;
	}
	_fd_io[_fd]._handle = INVALID_HANDLE_VALUE;
	_fd_io[_fd]._free = 1;
	return 0;
}

int close_fd(int _fd)
{
	EnterCriticalSection(&_fd_critical);
	int status = close_fd_internal(_fd);
	LeaveCriticalSection(&_fd_critical);
	return status;
}

///////////////////////////////////////
// Getters
///////////////////////////////////////
static HANDLE get_fd_handle_internal(int _fd)
{
	return _fd_io[_fd]._handle;
}

HANDLE get_fd_handle(int _fd)
{
	EnterCriticalSection(&_fd_critical);
	HANDLE handle = get_fd_handle_internal(_fd);
	LeaveCriticalSection(&_fd_critical);
	return handle;
}

static int get_fd_flags_internal(int _fd)
{
	return _fd_io[_fd]._flags;
}

int get_fd_flags(int _fd)
{
	int flags;
	EnterCriticalSection(&_fd_critical);
	flags = get_fd_flags_internal(_fd);
	LeaveCriticalSection(&_fd_critical);
	return flags;
}

static enum handle_type get_fd_type_internal(int _fd)
{
	if (validate_fd_internal(_fd))
		return _fd_io[_fd]._type;
	else
		return INVALID_HANDLE;
}

enum handle_type get_fd_type(int _fd)
{
	enum handle_type type;
	EnterCriticalSection(&_fd_critical);
	type = get_fd_type_internal(_fd);
	LeaveCriticalSection(&_fd_critical);
	return type;
}

static const wchar_t *get_path_internal(int _fd)
{
	return (const wchar_t *)_fd_io[_fd]._path;
}

const wchar_t *get_fd_path(int _fd)
{
	EnterCriticalSection(&_fd_critical);
	const wchar_t *wpath = get_path_internal(_fd);
	LeaveCriticalSection(&_fd_critical);
	return wpath;
}

///////////////////////////////////////
// Setters
///////////////////////////////////////
static void set_fd_handle_internal(int _fd, HANDLE _handle)
{
	_fd_io[_fd]._handle = _handle;
}

void set_fd_handle(int _fd, HANDLE _handle)
{
	EnterCriticalSection(&_fd_critical);
	set_fd_handle_internal(_fd, _handle);
	LeaveCriticalSection(&_fd_critical);
}

static void set_fd_flags_internal(int _fd, int _flags)
{
	_fd_io[_fd]._flags = _flags;
}

void set_fd_flags(int _fd, int _flags)
{
	EnterCriticalSection(&_fd_critical);
	set_fd_flags_internal(_fd, _flags);
	LeaveCriticalSection(&_fd_critical);
}

static void set_fd_type_internal(int _fd, enum handle_type _type)
{
	_fd_io[_fd]._type = _type;
}

void set_fd_type(int _fd, enum handle_type _type)
{
	EnterCriticalSection(&_fd_critical);
	set_fd_type_internal(_fd, _type);
	LeaveCriticalSection(&_fd_critical);
}

static void set_fd_path_internal(int _fd, const wchar_t *_path)
{
	HANDLE file = get_fd_handle_internal(_fd);
	if (file != NULL)
	{
		wchar_t temp_buf[MAX_PATH];
		int length = GetFinalPathNameByHandle(file, temp_buf, MAX_PATH, VOLUME_NAME_DOS);
		if (length == 0) // This will be true for a character device
		{
			wcscpy(_fd_io[_fd]._path, _path);
		}
		else
		{
			for (int i = 6; i < length; i++) // \\?\C:
			{
				if (temp_buf[i] == L'\\')
				{
					temp_buf[i] = L'/';
				}
			}
			wcscpy(_fd_io[_fd]._path, temp_buf + 4);
		}
	}
	else
	{
		wcscpy(_fd_io[_fd]._path, _path);
	}
}

void set_fd_path(int _fd, const wchar_t *_path)
{
	EnterCriticalSection(&_fd_critical);
	set_fd_path_internal(_fd, _path);
	LeaveCriticalSection(&_fd_critical);
}

static void add_fd_flags_internal(int _fd, int _flags)
{
	_fd_io[_fd]._flags |= _flags;
}

void add_fd_flags(int _fd, int _flags)
{
	EnterCriticalSection(&_fd_critical);
	add_fd_flags_internal(_fd, _flags);
	LeaveCriticalSection(&_fd_critical);
}

///////////////////////////////////////
// Validators
///////////////////////////////////////
static bool validate_fd_internal(int _fd)
{
	if (_fd >= _fd_table_size)
		return false;
	if (_fd_io[_fd]._free)
		return false;
	return true;
}

bool validate_fd(int _fd)
{
	EnterCriticalSection(&_fd_critical);
	bool condition = validate_fd_internal(_fd);
	LeaveCriticalSection(&_fd_critical);
	return condition;
}
