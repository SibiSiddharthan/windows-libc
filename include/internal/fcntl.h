/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef FCNTL_INTERNAL_H
#define FCNTL_INTERNAL_H

#include <sys/types.h>
#include <windows.h>
#include <wchar.h>
#include <stdbool.h>

typedef void *HANDLE;

enum handle_type
{
	NULL_HANDLE,
	CONSOLE_HANDLE,
	FILE_HANDLE,
	DIRECTORY_HANDLE,
	PIPE_HANDLE,
	INVALID_HANDLE = -1 // for errors
};

struct fd_table
{
	HANDLE _handle;
	int _flags;
	enum handle_type _type;
	bool _free;
	wchar_t _path[260];
};

extern struct fd_table *_fd_io;
extern size_t _fd_table_size;
extern CRITICAL_SECTION _fd_critical;

// Initialization and cleanup functions
void init_fd_table();
void cleanup_fd_table();

// Primary functions
// Create an entry in the fd table with the following values, Return the lowest fd possible
int register_to_fd_table(HANDLE _h, const wchar_t *_path, enum handle_type _type, int _flags);

// Update the fd with these values
void update_fd_table(int _fd, HANDLE _h, const wchar_t *_path, enum handle_type _type, int _flags);

// Create an entry in the fd table with the following values
void insert_into_fd_table(int _fd, HANDLE _h, const wchar_t *_path, enum handle_type _type, int _flags);

// Remove the file descriptor from the table without closing the handle
// Used by the hooks to stdio
void unregister_from_fd_table(HANDLE _h);

// Remove the file descriptor from the table and close it's handle
int close_fd(int _fd);

// Return the file descriptor corresponding to the given handle
int get_fd(HANDLE _h);

// Getters
HANDLE get_fd_handle(int _fd);
int get_fd_flags(int _fd);
enum handle_type get_fd_type(int _fd);
const wchar_t *get_fd_path(int _fd);

// Setters
void set_fd_handle(int _fd, HANDLE _handle);
void set_fd_flags(int _fd, int _flags);
void set_fd_type(int _fd, enum handle_type _type);
void set_fd_path(int _fd, const wchar_t *_path);

// Add flags to the file descriptor
void add_fd_flags(int _fd, int _flags);

// Validators
// Return true if we have an entry
bool validate_fd(int _fd);

// Few helper functions used by many functions
wchar_t *get_absolute_ntpath(int dirfd, const char *path);
HANDLE just_open(const wchar_t *u16_ntpath, ACCESS_MASK access, ULONG attributes, ULONG disposition, ULONG options);

#define VALIDATE_PATH(path, error)       \
	if (path == NULL || path[0] == '\0') \
	{                                    \
		errno = error;                   \
		return -1;                       \
	}

#define IS_ABSOLUTE_PATH(path) (((isalpha(path[0])) && (path[1] == ':')))

#define VALIDATE_DIRFD(dirfd)                                        \
	if (dirfd != AT_FDCWD && get_fd_type(dirfd) != DIRECTORY_HANDLE) \
	{                                                                \
		errno = ENOTDIR;                                             \
		return -1;                                                   \
	}

#define VALIDATE_PATH_AND_DIRFD(path, dirfd) \
	VALIDATE_PATH(path, ENOENT);             \
	if (!IS_ABSOLUTE_PATH(path))             \
	{                                        \
		VALIDATE_DIRFD(dirfd)                \
	}

#endif
