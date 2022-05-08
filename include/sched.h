/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_SCHED_H
#define WLIBC_SCHED_H

#include <wlibc.h>
#include <sys/types.h>
#include <time.h>

_WLIBC_BEGIN_DECLS

struct sched_param
{
	int sched_priority;
};

typedef struct _cpu_set_t
{
	int num_groups;
	int num_cpus;
	unsigned long long group_mask[1];
} cpu_set_t;

// Scheduling algorithms
// In Windows these denote the priority classes.
#define SCHED_IDLE     1 // PROCESS_PRIORITY_CLASS_IDLE
#define SCHED_RR       2 // PROCESS_PRIORITY_CLASS_NORMAL
#define SCHED_FIFO     3 // PROCESS_PRIORITY_CLASS_HIGH
#define SCHED_BATCH    5 // PROCESS_PRIORITY_CLASS_BELOW_NORMAL
#define SCHED_SPORADIC 6 // PROCESS_PRIORITY_CLASS_ABOVE_NORMAL

#define SCHED_MAX_PRIORITY 2
#define SCHED_MIN_PRIORITY -2

WLIBC_API int wlibc_sched_getparam(pid_t pid, struct sched_param *param);
WLIBC_API int wlibc_sched_setparam(pid_t pid, const struct sched_param *param);

WLIBC_INLINE int sched_getparam(pid_t pid, struct sched_param *param)
{
	return wlibc_sched_getparam(pid, param);
}
WLIBC_INLINE int sched_setparam(pid_t pid, const struct sched_param *param)
{
	return wlibc_sched_setparam(pid, param);
}

WLIBC_API int wlibc_sched_getscheduler(pid_t pid);
WLIBC_API int wlibc_sched_setscheduler(pid_t pid, int policy, const struct sched_param *param);

WLIBC_INLINE int sched_getscheduler(pid_t pid)
{
	return wlibc_sched_getscheduler(pid);
}

WLIBC_INLINE int sched_setscheduler(pid_t pid, int policy, const struct sched_param *param)
{
	return wlibc_sched_setscheduler(pid, policy, param);
}

WLIBC_API int wlibc_sched_yield(void);

WLIBC_INLINE int sched_yield(void)
{
	return wlibc_sched_yield();
}

// In Windows we can change the priority of a process/thread by two levels (increase or decrease)
// of its current priority class.
WLIBC_INLINE int sched_get_priority_max(int algorithm /*unused*/)
{
	return SCHED_MAX_PRIORITY;
}

WLIBC_INLINE int sched_get_priority_min(int algorithm /*unused*/)
{
	return SCHED_MIN_PRIORITY;
}

WLIBC_API int wlibc_sched_getaffinity(pid_t pid, size_t cpusetsize, cpu_set_t *cpuset);
WLIBC_API int wlibc_sched_setaffinity(pid_t pid, size_t cpusetsize, const cpu_set_t *cpuset);

WLIBC_INLINE int sched_getaffinity(pid_t pid, size_t cpusetsize, cpu_set_t *cpuset)
{
	return wlibc_sched_getaffinity(pid, cpusetsize, cpuset);
}

WLIBC_INLINE int sched_setaffinity(pid_t pid, size_t cpusetsize, const cpu_set_t *cpuset)
{
	return wlibc_sched_setaffinity(pid, cpusetsize, cpuset);
}

WLIBC_API size_t wlibc_cpu_alloc_size(int num_cpus);
WLIBC_API cpu_set_t *wlibc_cpu_alloc(int num_cpus);
WLIBC_API void wlibc_cpu_free(cpu_set_t *set);

WLIBC_INLINE size_t CPU_ALLOC_SIZE(int num_cpus)
{
	return wlibc_cpu_alloc_size(num_cpus);
}

WLIBC_INLINE cpu_set_t *CPU_ALLOC(int num_cpus)
{
	return wlibc_cpu_alloc(num_cpus);
}

WLIBC_INLINE void CPU_FREE(cpu_set_t *set)
{
	wlibc_cpu_free(set);
}

WLIBC_API void wlibc_cpu_zero(cpu_set_t *set);
WLIBC_API void wlibc_cpu_set(int cpu, cpu_set_t *set);
WLIBC_API void wlibc_cpu_clr(int cpu, cpu_set_t *set);
WLIBC_API int wlibc_cpu_isset(int cpu, cpu_set_t *set);
WLIBC_API int wlibc_cpu_count(cpu_set_t *set);
WLIBC_API void wlibc_cpu_and(cpu_set_t *destset, cpu_set_t *srcset1, cpu_set_t *srcset2);
WLIBC_API void wlibc_cpu_or(cpu_set_t *destset, cpu_set_t *srcset1, cpu_set_t *srcset2);
WLIBC_API void wlibc_cpu_xor(cpu_set_t *destset, cpu_set_t *srcset1, cpu_set_t *srcset2);
WLIBC_API int wlibc_cpu_equal(cpu_set_t *set1, cpu_set_t *set2);

WLIBC_INLINE void CPU_ZERO(cpu_set_t *set)
{
	wlibc_cpu_zero(set);
}

WLIBC_INLINE void CPU_SET(int cpu, cpu_set_t *set)
{
	wlibc_cpu_set(cpu, set);
}

WLIBC_INLINE void CPU_CLR(int cpu, cpu_set_t *set)
{
	wlibc_cpu_clr(cpu, set);
}

WLIBC_INLINE int CPU_ISSET(int cpu, cpu_set_t *set)
{
	return wlibc_cpu_isset(cpu, set);
}

WLIBC_INLINE int CPU_COUNT(cpu_set_t *set)
{
	return wlibc_cpu_count(set);
}

WLIBC_INLINE void CPU_AND(cpu_set_t *destset, cpu_set_t *srcset1, cpu_set_t *srcset2)
{
	wlibc_cpu_and(destset, srcset1, srcset2);
}

WLIBC_INLINE void CPU_OR(cpu_set_t *destset, cpu_set_t *srcset1, cpu_set_t *srcset2)
{
	wlibc_cpu_or(destset, srcset1, srcset2);
}

WLIBC_INLINE void CPU_XOR(cpu_set_t *destset, cpu_set_t *srcset1, cpu_set_t *srcset2)
{
	wlibc_cpu_xor(destset, srcset1, srcset2);
}

WLIBC_INLINE int CPU_EQUAL(cpu_set_t *set1, cpu_set_t *set2)
{
	return wlibc_cpu_equal(set1, set2);
}

WLIBC_INLINE void CPU_ZERO_S(size_t setsize /*unused*/, cpu_set_t *set)
{
	wlibc_cpu_zero(set);
}

WLIBC_INLINE void CPU_SET_S(int cpu, size_t setsize /*unused*/, cpu_set_t *set)
{
	wlibc_cpu_set(cpu, set);
}

WLIBC_INLINE void CPU_CLR_S(int cpu, size_t setsize /*unused*/, cpu_set_t *set)
{
	wlibc_cpu_clr(cpu, set);
}

WLIBC_INLINE int CPU_ISSET_S(int cpu, size_t setsize /*unused*/, cpu_set_t *set)
{
	return wlibc_cpu_isset(cpu, set);
}

WLIBC_INLINE int CPU_COUNT_S(size_t setsize /*unused*/, cpu_set_t *set)
{
	return wlibc_cpu_count(set);
}

WLIBC_INLINE void CPU_AND_S(size_t setsize /*unused*/, cpu_set_t *destset, cpu_set_t *srcset1, cpu_set_t *srcset2)
{
	wlibc_cpu_and(destset, srcset1, srcset2);
}

WLIBC_INLINE void CPU_OR_S(size_t setsize /*unused*/, cpu_set_t *destset, cpu_set_t *srcset1, cpu_set_t *srcset2)
{
	wlibc_cpu_or(destset, srcset1, srcset2);
}

WLIBC_INLINE void CPU_XOR_S(size_t setsize /*unused*/, cpu_set_t *destset, cpu_set_t *srcset1, cpu_set_t *srcset2)
{
	wlibc_cpu_xor(destset, srcset1, srcset2);
}

WLIBC_INLINE int CPU_EQUAL_S(size_t setsize /*unused*/, cpu_set_t *set1, cpu_set_t *set2)
{
	return wlibc_cpu_equal(set1, set2);
}

_WLIBC_END_DECLS

#endif
