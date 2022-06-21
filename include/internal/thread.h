/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_THREAD_INTERNAL_H
#define WLIBC_THREAD_INTERNAL_H

#include <internal/nt.h>
#include <signal.h>

typedef void *(*thread_start_t)(void *);
typedef void (*dtor_t)(void *);
typedef void (*cleanup_t)(void *);

extern DWORD _wlibc_threadinfo_index;
extern ULONGLONG _wlibc_tls_bitmap;
extern dtor_t _wlibc_tls_destructors[64];

typedef struct _tls_entry
{
	void *value;
} tls_entry;

typedef struct _cleanup_entry
{
	cleanup_t routine;
	void *arg;
} cleanup_entry;

typedef struct _threadinfo
{
	HANDLE handle;
	DWORD id;
	sigset_t sigmask;
	sigset_t pending;
	DWORD cancelstate;
	DWORD canceltype;
	thread_start_t routine;
	void *args;
	void *result;
	DWORD cleanup_slots_allocated;
	DWORD cleanup_slots_used;
	cleanup_entry *cleanup_entries;
	tls_entry slots[64];
} threadinfo;

void threads_init(void);
void threads_cleanup(void);
void cleanup_tls(threadinfo *tinfo);
void execute_cleanup(threadinfo *tinfo);

#endif
