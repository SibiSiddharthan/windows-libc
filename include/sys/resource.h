/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_SYS_RESOURCE_H
#define WLIBC_SYS_RESOURCE_H

#include <wlibc.h>
#include <sys/time.h>

_WLIBC_BEGIN_DECLS

typedef unsigned long long rlim_t;

#define RLIM_INFINITY  -1ull         // no limit
#define RLIM_SAVED_CUR RLIM_INFINITY // saved soft limit
#define RLIM_SAVED_MAX RLIM_INFINITY // saved hard limit

// rlimit resource
#define RLIMIT_CORE   0 // limit on size of core file
#define RLIMIT_CPU    1 // limit on CPU time per process
#define RLIMIT_DATA   2 // limit on data segment size
#define RLIMIT_FSIZE  3 // limit on file size
#define RLIMIT_NOFILE 4 // limit on number of open files
#define RLIMIT_STACK  5 // limit on stack size
#define RLIMIT_AS     6 // limit on address space size

// min/max nice values
#define PRIO_MIN -15
#define PRIO_MAX 15

// priority which
#define PRIO_PROCESS 0 // Current process
#define PRIO_PGRP    1 // Current process group
#define PRIO_USER    2 // Current user (Unsupported)

// rusage who
#define RUSAGE_SELF     0  // Calling process
#define RUSAGE_THREAD   1  // Calling thread
#define RUSAGE_CHILDREN 2 // Children of process (Unsupported)

struct rlimit
{
	rlim_t rlim_cur; // current soft limit
	rlim_t rlim_max; // hard limit
};

struct rusage
{
	struct timeval ru_utime;       // User time
	struct timeval ru_stime;       // System time
	unsigned long long ru_maxrss;  // Peak working set size
	unsigned long long ru_ixrss;   // Shared memory size
	unsigned long long ru_idrss;   // Private memory size
	unsigned long long ru_isrss;   // Private memory size
	unsigned long long ru_minflt;  // Page reclaims (soft page faults)
	unsigned long long ru_majflt;  // Page faults (hard page faults)
	unsigned long long ru_nswap;   // Swap file usage
	unsigned long long ru_inblock; // File write operations
	unsigned long long ru_oublock; // File read operations
};

WLIBC_API int wlibc_getrlimit(int resource, struct rlimit *rlim);
WLIBC_API int wlibc_setrlimit(int resource, const struct rlimit *rlim);

WLIBC_INLINE int getrlimit(int resource, struct rlimit *rlim)
{
	return wlibc_getrlimit(resource, rlim);
}

WLIBC_INLINE int setrlimit(int resource, struct rlimit *rlim)
{
	return wlibc_setrlimit(resource, rlim);
}

WLIBC_API int wlibc_getpriority(int which, id_t who);
WLIBC_API int wlibc_setpriority(int which, id_t who, int priority);

WLIBC_INLINE int getpriority(int which, id_t who)
{
	return wlibc_getpriority(which, who);
}

WLIBC_INLINE int setpriority(int which, id_t who, int priority /*actually nice*/)
{
	return wlibc_setpriority(which, who, priority);
}

WLIBC_API int wlibc_getrusage(int who, struct rusage *usage);
WLIBC_INLINE int getrusage(int who, struct rusage *usage)
{
	return wlibc_getrusage(who, usage);
}

_WLIBC_END_DECLS

#endif
