/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/error.h>
#include <internal/fcntl.h>
#include <errno.h>
#include <fcntl.h>

fdinfo *_wlibc_fd_table = NULL;
size_t _wlibc_fd_table_size = 0;
unsigned int _wlibc_fd_sequence = 0;
RTL_SRWLOCK _wlibc_fd_table_srwlock;

// Declaration of static functions
static int internal_insert_fd(int index, HANDLE _h, handle_t _type, int _flags);
static int register_to_fd_table_internal(HANDLE _h, handle_t _type, int _flags);
static int insert_into_fd_table_internal(int _fd, HANDLE _h, handle_t _type, int _flags);
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

handle_t determine_handle_type(HANDLE handle)
{
	NTSTATUS status;
	IO_STATUS_BLOCK io;
	FILE_FS_DEVICE_INFORMATION device_info;

	status = NtQueryVolumeInformationFile(handle, &io, &device_info, sizeof(FILE_FS_DEVICE_INFORMATION), FileFsDeviceInformation);
	if (status != STATUS_SUCCESS)
	{
		return INVALID_HANDLE;
	}

	switch (device_info.DeviceType)
	{
	case FILE_DEVICE_NULL:
		return NULL_HANDLE;
	case FILE_DEVICE_CONSOLE:
		return CONSOLE_HANDLE;
	case FILE_DEVICE_DISK:
	{
		// Check whether the inherited handle is a directory or a file.
		FILE_ATTRIBUTE_TAG_INFORMATION tag_info;

		status = NtQueryInformationFile(handle, &io, &tag_info, sizeof(FILE_ATTRIBUTE_TAG_INFORMATION), FileAttributeTagInformation);
		if (status != STATUS_SUCCESS)
		{
			// Assume a file handle in case of failure.
			return FILE_HANDLE;
		}

		if (tag_info.FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			return DIRECTORY_HANDLE;
		}

		return FILE_HANDLE;
	}
	case FILE_DEVICE_NAMED_PIPE:
		return PIPE_HANDLE;
	default:
		return INVALID_HANDLE;
	}
}

int determine_handle_flags(HANDLE handle)
{
	NTSTATUS status;
	IO_STATUS_BLOCK io;
	FILE_ACCESS_INFORMATION access_info;
	int flags = 0;

	status = NtQueryInformationFile(handle, &io, &access_info, sizeof(FILE_ACCESS_INFORMATION), FileAccessInformation);
	if (status != STATUS_SUCCESS)
	{
		return O_RDONLY; // Can't say anything about the access, assume O_RDONLY.
	}

	// Check for append flag.
	if (access_info.AccessFlags & FILE_APPEND_DATA)
	{
		flags |= O_APPEND | O_WRONLY;
	}

	// Do the generic permissions first.
	if (access_info.AccessFlags & (GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE | GENERIC_ALL))
	{
		if ((access_info.AccessFlags & GENERIC_ALL) ||
			(access_info.AccessFlags & (GENERIC_READ | GENERIC_WRITE)) == (GENERIC_READ | GENERIC_WRITE))
		{
			flags |= O_RDWR;
		}
		else if (access_info.AccessFlags & GENERIC_WRITE)
		{
			flags |= O_WRONLY;
		}
		else if (access_info.AccessFlags & GENERIC_READ)
		{
			flags |= O_RDONLY;
		}
	}

	if ((access_info.AccessFlags & (FILE_READ_DATA | FILE_WRITE_DATA)) == (FILE_READ_DATA | FILE_WRITE_DATA))
	{
		flags |= O_RDWR;
	}
	else if (access_info.AccessFlags & FILE_WRITE_DATA)
	{
		flags |= O_WRONLY;
	}
	else if (access_info.AccessFlags & FILE_READ_DATA)
	{
		flags |= O_RDONLY;
	}

	// Remove O_WRONLY if O_RDWR is present.
	if ((flags & (O_WRONLY | O_RDWR)) == (O_WRONLY | O_RDWR))
	{
		flags &= ~O_WRONLY;
	}

	return flags;
}

// Opening the console handle should not fail as we are only doing it if there is a console attached.
HANDLE open_conin(void)
{
	HANDLE handle = NULL;
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
	HANDLE handle = NULL;
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

// Opening the mount manager should not fail.
HANDLE open_mountmgr(void)
{
	UNICODE_STRING name;
	OBJECT_ATTRIBUTES object;
	IO_STATUS_BLOCK io;
	HANDLE handle = NULL;

	name.Buffer = MOUNTMGR_DEVICE_NAME;
	name.Length = 50;
	name.MaximumLength = 52;

	InitializeObjectAttributes(&object, &name, 0, NULL, NULL);
	NtCreateFile(&handle, SYNCHRONIZE, &object, &io, NULL, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, FILE_OPEN, 0, NULL, 0);
	return handle;
}

void initialize_std_handles(HANDLE handle, int index, bool console_subsystem, bool output)
{
	handle_t inherited_handle_type;

	inherited_handle_type = determine_handle_type(handle);
	if (inherited_handle_type != INVALID_HANDLE)
	{
		_wlibc_fd_table[index].handle = handle;
		_wlibc_fd_table[index].type = inherited_handle_type;
		_wlibc_fd_table[index].flags = determine_handle_flags(handle);
		_wlibc_fd_table[index].sequence = ++_wlibc_fd_sequence;
	}
	else if (console_subsystem)
	{
		_wlibc_fd_table[index].handle = output == true ? open_conout() : open_conin();
		if (_wlibc_fd_table[index].handle != NULL)
		{
			_wlibc_fd_table[index].type = CONSOLE_HANDLE;
			_wlibc_fd_table[index].flags = output == true ? O_WRONLY : O_RDONLY;
			_wlibc_fd_table[index].sequence = ++_wlibc_fd_sequence;
		}
		else
		{
			_wlibc_fd_table[index].handle = NULL;
		}
	}
}

#define FOPEN_FLAG      0x01 // file handle open
#define FEOFLAG_FLAG    0x02 // end of file has been encountered
#define FCRLF_FLAG      0x04 // CR-LF across read buffer (in text mode)
#define FPIPE_FLAG      0x08 // file handle refers to a pipe
#define FNOINHERIT_FLAG 0x10 // file handle opened O_NOINHERIT
#define FAPPEND_FLAG    0x20 // file handle opened O_APPEND
#define FNULL_FLAG      0x40 // file handle refers to a device (Originally FDEV)
#define FCONSOLE_FLAG   0x80 // file handle is in text mode  (Originally FTEXT)

///////////////////////////////////////
// Initialization and cleanup functions
///////////////////////////////////////
void init_fd_table(void)
{
	UNICODE_STRING *data;
	bool console_subsystem = true;

	RtlInitializeSRWLock(&_wlibc_fd_table_srwlock);

	if (NtCurrentPeb()->ProcessParameters->ConsoleHandle == 0 || NtCurrentPeb()->ProcessParameters->ConsoleHandle == (HANDLE)-1)
	{
		console_subsystem = false;
	}

	// Always inherit the standard handles from these parameters.
	HANDLE hin = NtCurrentPeb()->ProcessParameters->StandardInput;
	HANDLE hout = NtCurrentPeb()->ProcessParameters->StandardOutput;
	HANDLE herr = NtCurrentPeb()->ProcessParameters->StandardError;
	// For the rest use the 'RuntimeData' parameter of 'RTL_USER_PROCESS_PARAMETERS'.
	// This is the same as the 'lpReserved2' and 'cbReserved2' parameter of STARTUPINFO.
	data = &NtCurrentPeb()->ProcessParameters->RuntimeData;

	// This first 4 bytes say the number of handles inherited.
	DWORD number_of_handles_inherited = data->Buffer == NULL ? 0 : *(DWORD *)data->Buffer;

	_wlibc_fd_table_size = 4;
	// Set the initial size of the fd tables to be the nearest 2 power enough to fit in all the handles.
	while (_wlibc_fd_table_size < number_of_handles_inherited)
	{
		_wlibc_fd_table_size *= 2;
	}

	_wlibc_fd_table = (fdinfo *)RtlAllocateHeap(NtCurrentProcessHeap(), 0, sizeof(fdinfo) * _wlibc_fd_table_size);

	// Exit the process if the initialization routine fails.
	if (_wlibc_fd_table == NULL)
	{
		RtlExitUserProcess(STATUS_NO_MEMORY);
	}

	// Mark all handles as invalid at the start.
	for (size_t i = 0; i < _wlibc_fd_table_size; ++i)
	{
		_wlibc_fd_table[i].handle = NULL;
	}

	// Standard Input,Output,Error.
	initialize_std_handles(hin, 0, console_subsystem, false);
	initialize_std_handles(hout, 1, console_subsystem, true);
	initialize_std_handles(herr, 2, console_subsystem, true);

	// Cygwin/MSYS gives the same handle as stdout and stderr, duplicate the handle.
	if ((_wlibc_fd_table[1].handle != NULL && _wlibc_fd_table[2].handle != NULL) &&
		_wlibc_fd_table[1].handle == _wlibc_fd_table[2].handle)
	{
		NTSTATUS status;
		HANDLE new_stderr;

		status = NtDuplicateObject(NtCurrentProcess(), _wlibc_fd_table[1].handle, NtCurrentProcess(), &new_stderr, 0, 0,
								   DUPLICATE_SAME_ACCESS | DUPLICATE_SAME_ATTRIBUTES);
		if (status == STATUS_SUCCESS)
		{
			// Don't do anything in case of failure
			_wlibc_fd_table[2].handle = new_stderr;
		}
	}

	// Initialize the remaining inherited file descriptors if any.
	if (number_of_handles_inherited != 0)
	{
		for (DWORD i = 3; i < number_of_handles_inherited; ++i)
		{
			handle_t inherited_handle_type;
			// For each of the handles a handle flag is passed as an UCHAR after the first 4 bytes.
			UCHAR handle_flag = *(UCHAR *)((CHAR *)data->Buffer + sizeof(DWORD) + i);
			// After all the handle flags the handles values are passed.
			HANDLE inherited_handle = *(HANDLE *)((CHAR *)data->Buffer + sizeof(DWORD) + number_of_handles_inherited + i * sizeof(HANDLE));

			// msvcrt sets invalid handles to INVALID_HANDLE_VALUE to inherited processes.
			if ((handle_flag & FOPEN_FLAG) == 0 || inherited_handle == INVALID_HANDLE_VALUE)
			{
				_wlibc_fd_table[i].handle = NULL;
				continue;
			}

			inherited_handle_type = determine_handle_type(inherited_handle);
			if (inherited_handle_type == INVALID_HANDLE)
			{
				_wlibc_fd_table[i].handle = NULL;
				continue;
			}

			// Valid handle. Don't trust the other flags that are passed, query the kernel for the actual values.
			_wlibc_fd_table[i].handle = inherited_handle;
			_wlibc_fd_table[i].flags = determine_handle_flags(inherited_handle);
			_wlibc_fd_table[i].type = inherited_handle_type;
			_wlibc_fd_table[i].sequence = ++_wlibc_fd_sequence;
		}
	}
}

// Not worrying about open handles (not closed by the user)
void cleanup_fd_table(void)
{
	RtlFreeHeap(NtCurrentProcessHeap(), 0, _wlibc_fd_table);
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

	switch (index)
	{
	case 0:
		NtCurrentPeb()->ProcessParameters->StandardInput = _h;
		break;
	case 1:
		NtCurrentPeb()->ProcessParameters->StandardOutput = _h;
		break;
	case 2:
		NtCurrentPeb()->ProcessParameters->StandardError = _h;
		break;
	default:
		break;
	}

	return index;
}

static int register_to_fd_table_internal(HANDLE _h, handle_t _type, int _flags)
{
	bool is_there_free_space = false;
	int index = -1;
	int fd = -1;
	for (size_t i = 0; i < _wlibc_fd_table_size; i++)
	{
		if (_wlibc_fd_table[i].handle == NULL)
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
		void *temp = (fdinfo *)RtlReAllocateHeap(NtCurrentProcessHeap(), 0, _wlibc_fd_table, sizeof(fdinfo) * _wlibc_fd_table_size * 2);
		if (temp == NULL)
		{
			errno = ENOMEM;
			return -1;
		}

		_wlibc_fd_table = temp;

		fd = internal_insert_fd((int)_wlibc_fd_table_size, _h, _type, _flags);

		for (size_t i = _wlibc_fd_table_size + 1; i < _wlibc_fd_table_size * 2; i++)
		{
			_wlibc_fd_table[i].handle = NULL;
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

static int insert_into_fd_table_internal(int _fd, HANDLE _h, handle_t _type, int _flags)
{
	// grow the table
	if (_fd >= (int)_wlibc_fd_table_size)
	{
		void *temp = (fdinfo *)RtlReAllocateHeap(NtCurrentProcessHeap(), 0, _wlibc_fd_table,
												 sizeof(fdinfo) * _fd * 2); // Allocate double the requested fd number

		if (temp == NULL)
		{
			errno = ENOMEM;
			return -1;
		}

		_wlibc_fd_table = temp;

		for (int i = (int)_wlibc_fd_table_size; i < _fd * 2; ++i)
		{
			_wlibc_fd_table[i].handle = NULL;
		}

		_wlibc_fd_table_size = _fd * 2;
	}

	internal_insert_fd(_fd, _h, _type, _flags);
	return _fd;
}

int insert_into_fd_table(int _fd, HANDLE _h, handle_t _type, int _flags)
{
	int fd;
	EXCLUSIVE_LOCK_FD_TABLE();
	fd = insert_into_fd_table_internal(_fd, _h, _type, _flags);
	EXCLUSIVE_UNLOCK_FD_TABLE();
	return fd;
}

static void unregister_from_fd_table_internal(HANDLE _h)
{
	for (size_t i = 0; i < _wlibc_fd_table_size; i++)
	{
		if (_wlibc_fd_table[i].handle != INVALID_HANDLE_VALUE && _wlibc_fd_table[i].handle == _h)
		{
			// Calling this function after the handle has been closed
			_wlibc_fd_table[i].handle = NULL;
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
	_wlibc_fd_table[_fd].handle = NULL;
	_wlibc_fd_table[_fd].type = INVALID_HANDLE;
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

void get_fdinfo(int fd, fdinfo *info)
{
	SHARED_LOCK_FD_TABLE();

	if (validate_fd_internal(fd))
	{
		memcpy(info, &_wlibc_fd_table[fd], sizeof(fdinfo));
	}
	else
	{
		info->handle = NULL;
		info->type = INVALID_HANDLE;
	}

	SHARED_UNLOCK_FD_TABLE();
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
	if (_wlibc_fd_table[_fd].handle == NULL)
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
