/*
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT
*/

#ifndef WLIBC_SYS_RESOURCE_H
#define WLIBC_SYS_RESOURCE_H

#include <wlibc-macros.h>
#include <sys/time.h>

_WLIBC_BEGIN_DECLS

typedef unsigned long long rlim_t;

#define RLIM_INFINITY  -1ull         // no limit
#define RLIM_SAVED_CUR RLIM_INFINITY // saved soft limit
#define RLIM_SAVED_MAX RLIM_INFINITY // saved hard limit

#define RLIMIT_CORE   0 // limit on size of core file
#define RLIMIT_CPU    1 // limit on CPU time per process
#define RLIMIT_DATA   2 // limit on data segment size
#define RLIMIT_FSIZE  3 // limit on file size
#define RLIMIT_NOFILE 4 // limit on number of open files
#define RLIMIT_STACK  5 // limit on stack size
#define RLIMIT_AS     6 // limit on address space size

struct rlimit
{
	rlim_t rlim_cur; // current soft limit
	rlim_t rlim_max; // hard limit
};

struct rusage
{
	struct timeval ru_utime; // User time
	struct timeval ru_stime; // System time
};

WLIBC_API int wlibc_getrlimit(int resource, struct rlimit *rlim);

WLIBC_INLINE int getrlimit(int resource, struct rlimit *rlim)
{
	return wlibc_getrlimit(resource, rlim);
}

WLIBC_API int wlibc_setrlimit(int resource, const struct rlimit *rlim);

WLIBC_INLINE int setrlimit(int resource, struct rlimit *rlim)
{
	return wlibc_setrlimit(resource, rlim);
}

_WLIBC_END_DECLS

#endif
