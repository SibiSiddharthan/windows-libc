/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef FCNTL_INTERNAL_H
#define FCNTL_INTERNAL_H

#include <internal/nt.h>
#include <sys/types.h>
#include <stdbool.h>

typedef void *HANDLE;

typedef enum _handle_t
{
	NULL_HANDLE,
	CONSOLE_HANDLE,
	FILE_HANDLE,
	DIRECTORY_HANDLE,
	PIPE_HANDLE,
	INVALID_HANDLE = -1 // for errors
} handle_t;

struct fd_table
{
	HANDLE _handle;
	handle_t _type;
	int _flags;
	unsigned int _sequence;
};

extern struct fd_table *_fd_io;
extern size_t _fd_table_size;
extern unsigned int _fd_io_sequence;
extern CRITICAL_SECTION _fd_critical;

// Initialization and cleanup functions
void init_fd_table(void);
void cleanup_fd_table(void);

// Primary functions
// Create an entry in the fd table with the following values, Return the lowest fd possible
int register_to_fd_table(HANDLE _h, handle_t _type, int _flags);

// Update the fd with these values
void update_fd_table(int _fd, HANDLE _h, handle_t _type, int _flags);

// Create an entry in the fd table with the following values
void insert_into_fd_table(int _fd, HANDLE _h, handle_t _type, int _flags);

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
handle_t get_fd_type(int _fd);

// Setters
void set_fd_handle(int _fd, HANDLE _handle);
void set_fd_flags(int _fd, int _flags);
void set_fd_type(int _fd, handle_t _type);

// Add flags to the file descriptor
void add_fd_flags(int _fd, int _flags);

// Validators
// Return true if we have an entry
bool validate_fd(int _fd);

// Few helper functions used by many functions
// Ignore attributes and disposition here as they will be '0' and 'FILE_OPEN' respectively.
HANDLE just_open(int dirfd, const char *path, ACCESS_MASK access, ULONG options);
HANDLE just_open2(UNICODE_STRING *ntpath, ACCESS_MASK access, ULONG options);

#define VALIDATE_PATH(path, error, ret)  \
	if (path == NULL || path[0] == '\0') \
	{                                    \
		errno = error;                   \
		return ret;                      \
	}

#define IS_ABSOLUTE_PATH(path) (((isalpha(path[0])) && (path[1] == ':')))

#define VALIDATE_DIRFD(dirfd)               \
	if (dirfd != AT_FDCWD)                  \
	{                                       \
                                            \
		handle_t type = get_fd_type(dirfd); \
		if (type == INVALID_HANDLE)         \
		{                                   \
			errno = EBADF;                  \
			return -1;                      \
		}                                   \
		if (type != DIRECTORY_HANDLE)       \
		{                                   \
			errno = ENOTDIR;                \
			return -1;                      \
		}                                   \
	}

#define VALIDATE_PATH_AND_DIRFD(path, dirfd) \
	VALIDATE_PATH(path, ENOENT, -1);         \
	if (!IS_ABSOLUTE_PATH(path))             \
	{                                        \
		VALIDATE_DIRFD(dirfd)                \
	}

#endif
