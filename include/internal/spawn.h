/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_SPAWN_INTERNAL_H
#define WLIBC_SPAWN_INTERNAL_H

#include <internal/nt.h>
#include <stdbool.h>
#include <sys/types.h>

typedef struct _process_table
{
	HANDLE process_handle;
	pid_t process_id;
} process_table;

extern process_table *_wlibc_process_table;
extern pid_t _wlibc_process_table_size;
extern pid_t _wlibc_child_process_count;

extern RTL_SRWLOCK _wlibc_process_table_srwlock;

void process_init(void);
void process_cleanup(void);

bool is_child(pid_t pid);
HANDLE get_child_handle(pid_t pid);
unsigned int get_child_process_count();

void add_child(pid_t pid, HANDLE child);
void delete_child(HANDLE process_handle);

#define SHARED_LOCK_PROCESS_TABLE()      RtlAcquireSRWLockShared(&_wlibc_process_table_srwlock)
#define SHARED_UNLOCK_PROCESS_TABLE()    RtlReleaseSRWLockShared(&_wlibc_process_table_srwlock)
#define EXCLUSIVE_LOCK_PROCESS_TABLE()   RtlAcquireSRWLockExclusive(&_wlibc_process_table_srwlock)
#define EXCLUSIVE_UNLOCK_PROCESS_TABLE() RtlReleaseSRWLockExclusive(&_wlibc_process_table_srwlock)

#endif
