/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_SPAWN_INTERNAL_H
#define WLIBC_SPAWN_INTERNAL_H

#include <Windows.h>
#include <stdbool.h>
#include <sys/types.h>

typedef struct
{
	HANDLE process_handle;
	pid_t process_id;
} process_table;

extern process_table *_wlibc_process_table;
extern pid_t _wlibc_process_table_size;
extern pid_t _wlibc_child_process_count;

extern CRITICAL_SECTION _wlibc_process_critical;

void process_init(void);
void process_cleanup(void);

bool is_child(pid_t pid);
HANDLE get_child_handle(pid_t pid);
unsigned int get_child_process_count();

void add_child(pid_t pid, HANDLE child);
void delete_child(HANDLE process_handle);

#endif
