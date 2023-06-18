/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_SPAWN_INTERNAL_H
#define WLIBC_SPAWN_INTERNAL_H

#include <internal/nt.h>
#include <sys/types.h>

typedef struct _processinfo
{
	HANDLE handle;
	DWORD id;
} processinfo;

extern processinfo *_wlibc_process_table;
extern size_t _wlibc_process_table_size;
extern size_t _wlibc_child_process_count;

extern RTL_SRWLOCK _wlibc_process_table_srwlock;

void process_init(void);
void process_cleanup(void);

void get_processinfo(pid_t pid, processinfo *pinfo);

int add_child(DWORD id, HANDLE child);
void delete_child(DWORD id);

#define SHARED_LOCK_PROCESS_TABLE()      RtlAcquireSRWLockShared(&_wlibc_process_table_srwlock)
#define SHARED_UNLOCK_PROCESS_TABLE()    RtlReleaseSRWLockShared(&_wlibc_process_table_srwlock)
#define EXCLUSIVE_LOCK_PROCESS_TABLE()   RtlAcquireSRWLockExclusive(&_wlibc_process_table_srwlock)
#define EXCLUSIVE_UNLOCK_PROCESS_TABLE() RtlReleaseSRWLockExclusive(&_wlibc_process_table_srwlock)

#endif
