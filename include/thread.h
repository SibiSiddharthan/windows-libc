/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_THREAD_H
#define WLIBC_THREAD_H

#include <wlibc.h>
#include <sched.h>
#include <signal.h>
#include <time.h>

_WLIBC_BEGIN_DECLS

typedef void *thread_t;
typedef void *(*thread_start_t)(void *);
typedef void (*cleanup_t)(void *);

typedef struct _wlibc_thread_attr_t
{
	int detachstate;
	int suspendstate;
	int inherit;
	int policy;
	int priority;
	size_t stacksize;
	cpu_set_t *set;
} thread_attr_t;

typedef struct _wlibc_mutex_attr_t
{
	int shared;
	int type;
} mutex_attr_t;

typedef struct _wlibc_cond_attr_t
{
	int shared;
} cond_attr_t;

typedef struct _wlibc_barrier_attr_t
{
	int shared;
} barrier_attr_t;

typedef struct _wlibc_rwlock_attr_t
{
	int shared;
} rwlock_attr_t;

typedef union _wlibc_once_t {
	void *ptr;
} once_t;

typedef struct _wlibc_mutex_t
{
	unsigned int owner;
	unsigned int count;
	int type;
	void *handle;
} mutex_t;

typedef struct _wlibc_cond_t
{
	unsigned long *ptr;
	int waiting_threads;
	int queue_size;
	int queue_begin;
	int queue_end;
	int lock;
} cond_t;

typedef struct _wlibc_barrier_t
{
	unsigned int reserved[8];
} barrier_t;

typedef struct _wlibc_rwlock_t
{
	void *ptr;
	int lock;
} rwlock_t;

typedef unsigned long key_t;
typedef void (*dtor_t)(void *);

#define WLIBC_THREAD_INHERIT_SCHED  0
#define WLIBC_THREAD_EXPLICIT_SCHED 1

#define WLIBC_THREAD_SCOPE_SYSTEM  0
#define WLIBC_THREAD_SCOPE_PROCESS 1 // Unsupported

#define WLIBC_THREAD_CANCEL_ENABLE       0
#define WLIBC_THREAD_CANCEL_DISABLE      1
#define WLIBC_THREAD_CANCEL_ASYNCHRONOUS 0 // Immediate cancellation
#define WLIBC_THREAD_CANCEL_DEFERRED     1 // Deferred cancellation (Same as above in Windows)

#define WLIBC_THREAD_CANCELED  ((void *)(intptr_t)-1)
#define WLIBC_THREAD_CANCELLED WLIBC_THREAD_CANCELED

// clang-format off
#define WLIBC_THREAD_ONCE_INIT {0}
#define WLIBC_DTOR_ITERATIONS 1
// clang-format on

#define WLIBC_THREAD_JOINABLE 0 // Joinable thread.
#define WLIBC_THREAD_DETACHED 1 // Detached thread.

#define WLIBC_THREAD_RUNNING   0 // Running thread.
#define WLIBC_THREAD_SUSPENDED 1 // Suspended thread.

#define WLIBC_PROCESS_PRIVATE 0 // Private to a process.
#define WLIBC_PROCESS_SHARED  1 // Shareabled across processes.

#define WLIBC_MUTEX_NORMAL    0x0 // Plain mutex, infinite wait
#define WLIBC_MUTEX_RECURSIVE 0x1 // Recursive mutex
#define WLIBC_MUTEX_TIMED     0x2 // Waits can timeout

// Thread functions.
WLIBC_API int wlibc_thread_create(thread_t *thread, thread_attr_t *attributes, thread_start_t routine, void *arg);
WLIBC_API int wlibc_thread_detach(thread_t thread);
WLIBC_API int wlibc_thread_join(thread_t thread, void **result);
WLIBC_API int wlibc_thread_tryjoin(thread_t thread, void **result);
WLIBC_API int wlibc_thread_timedjoin(thread_t thread, void **result, const struct timespec *abstime);
WLIBC_API int wlibc_thread_equal(thread_t thread_a, thread_t thread_b);
WLIBC_API thread_t wlibc_thread_self(void);
WLIBC_API int wlibc_threadid(thread_t thread, pid_t *id);
WLIBC_API int wlibc_thread_sleep(const struct timespec *duration, struct timespec *remaining);
WLIBC_API WLIBC_NORETURN void wlibc_thread_exit(void *retval);
WLIBC_API int wlibc_thread_setcancelstate(int state, int *oldstate);
WLIBC_API int wlibc_thread_setcanceltype(int type, int *oldtype);
WLIBC_API int wlibc_thread_cancel(thread_t thread);
WLIBC_API void wlibc_thread_testcancel(void);
WLIBC_API void wlibc_thread_cleanup_push(cleanup_t routine, void *arg);
WLIBC_API void wlibc_thread_cleanup_pop(int execute);
WLIBC_API int wlibc_threadattr_init(thread_attr_t *attributes);
WLIBC_API int wlibc_threadattr_getdetachstate(const thread_attr_t *attributes, int *detachstate);
WLIBC_API int wlibc_threadattr_setdetachstate(thread_attr_t *attributes, int detachstate);
WLIBC_API int wlibc_threadattr_getsuspendstate(const thread_attr_t *attributes, int *suspendstate);
WLIBC_API int wlibc_threadattr_setsuspendstate(thread_attr_t *attributes, int suspendstate);
WLIBC_API int wlibc_threadattr_getstacksize(const thread_attr_t *restrict attributes, size_t *restrict stacksize);
WLIBC_API int wlibc_threadattr_setstacksize(thread_attr_t *attributes, size_t stacksize);
WLIBC_API int wlibc_threadattr_getscope(const thread_attr_t *restrict attributes, int *restrict scope);
WLIBC_API int wlibc_threadattr_setscope(thread_attr_t *attributes, int scope);
WLIBC_API int wlibc_threadattr_getinheritsched(thread_attr_t *attributes, int *inherit);
WLIBC_API int wlibc_threadattr_setinheritsched(thread_attr_t *attributes, int inherit);
WLIBC_API int wlibc_threadattr_getschedparam(const thread_attr_t *restrict attributes, struct sched_param *restrict param);
WLIBC_API int wlibc_threadattr_setschedparam(thread_attr_t *restrict attributes, const struct sched_param *restrict param);
WLIBC_API int wlibc_threadattr_getschedpolicy(const thread_attr_t *restrict attributes, int *restrict policy);
WLIBC_API int wlibc_threadattr_setschedpolicy(thread_attr_t *attributes, int policy);
WLIBC_API int wlibc_threadattr_getaffinity(const thread_attr_t *attributes, cpu_set_t *restrict cpuset);
WLIBC_API int wlibc_threadattr_setaffinity(thread_attr_t *attributes, const cpu_set_t *restrict cpuset);

WLIBC_API int wlibc_thread_resume(thread_t thread);
WLIBC_API int wlibc_thread_suspend(thread_t thread);
WLIBC_API int wlibc_thread_setschedparam(thread_t thread, int policy, const struct sched_param *param);
WLIBC_API int wlibc_thread_getschedparam(thread_t thread, int *restrict policy, struct sched_param *restrict param);
WLIBC_API int wlibc_thread_getschedpriority(thread_t thread, int *priority);
WLIBC_API int wlibc_thread_setschedpriority(thread_t thread, int priority);
WLIBC_API int wlibc_thread_getname(thread_t thread, char *buffer, size_t length);
WLIBC_API int wlibc_thread_setname(thread_t thread, const char *name);
WLIBC_API int wlibc_thread_getaffinity(thread_t thread, cpu_set_t *cpuset);
WLIBC_API int wlibc_thread_setaffinity(thread_t thread, const cpu_set_t *cpuset);
WLIBC_API int wlibc_thread_getconcurrency(void);
WLIBC_API int wlibc_thread_setconcurrency(int level);

WLIBC_API int wlibc_thread_kill(thread_t thread, int sig);

WLIBC_INLINE int wlibc_thread_sigmask(int how, const sigset_t *newset, sigset_t *oldset)
{
	return wlibc_sigprocmask(how, newset, oldset);
}

WLIBC_INLINE int wlibc_thread_yield(void)
{
	return wlibc_sched_yield();
}

// One time initialization.
WLIBC_API int wlibc_thread_once(once_t *control, void (*init)(void));

// Mutex functions.
WLIBC_API int wlibc_mutex_init(mutex_t *mutex, const mutex_attr_t *attributes);
WLIBC_API int wlibc_mutex_destroy(mutex_t *mutex);
WLIBC_API int wlibc_mutex_trylock(mutex_t *mutex);
WLIBC_API int wlibc_mutex_lock(mutex_t *mutex);
WLIBC_API int wlibc_mutex_timedlock(mutex_t *restrict mutex, const struct timespec *restrict abstime);
WLIBC_API int wlibc_mutex_unlock(mutex_t *mutex);
WLIBC_API int wlibc_mutexattr_init(mutex_attr_t *attributes);
WLIBC_API int wlibc_mutexattr_getpshared(const mutex_attr_t *restrict attributes, int *restrict pshared);
WLIBC_API int wlibc_mutexattr_setpshared(mutex_attr_t *attributes, int pshared);
WLIBC_API int wlibc_mutexattr_gettype(const mutex_attr_t *restrict attributes, int *restrict type);
WLIBC_API int wlibc_mutexattr_settype(mutex_attr_t *attributes, int type);

// Condition variable functions.
WLIBC_API int wlibc_cond_init(cond_t *restrict cond, const cond_attr_t *restrict attributes);
WLIBC_API int wlibc_cond_destroy(cond_t *cond);
WLIBC_API int wlibc_cond_signal(cond_t *cond);
WLIBC_API int wlibc_cond_broadcast(cond_t *cond);
WLIBC_API int wlibc_cond_wait(cond_t *restrict cond, mutex_t *restrict mutex);
WLIBC_API int wlibc_cond_timedwait(cond_t *restrict cond, mutex_t *restrict mutex, const struct timespec *restrict abstime);
WLIBC_API int wlibc_condattr_init(cond_attr_t *attributes);
WLIBC_API int wlibc_condattr_getpshared(const cond_attr_t *restrict attributes, int *restrict pshared);
WLIBC_API int wlibc_condattr_setpshared(cond_attr_t *attributes, int pshared);

// Barrier functions.
WLIBC_API int wlibc_barrier_init(barrier_t *restrict barrier, const barrier_attr_t *restrict attributes, unsigned int count);
WLIBC_API int wlibc_barrier_destroy(barrier_t *barrier);
WLIBC_API int wlibc_barrier_wait(barrier_t *barrier);
WLIBC_API int wlibc_barrierattr_init(barrier_attr_t *attributes);
WLIBC_API int wlibc_barrierattr_getpshared(const barrier_attr_t *restrict attributes, int *restrict pshared);
WLIBC_API int wlibc_barrierattr_setpshared(barrier_attr_t *attributes, int pshared);

// Reader-Writer lock functions.
WLIBC_API int wlibc_rwlock_init(rwlock_t *restrict rwlock, const rwlock_attr_t *restrict attributes);
WLIBC_API int wlibc_rwlock_destroy(rwlock_t *rwlock);
WLIBC_API int wlibc_rwlock_rdlock(rwlock_t *rwlock);
WLIBC_API int wlibc_rwlock_tryrdlock(rwlock_t *rwlock);
WLIBC_API int wlibc_rwlock_timedrdlock(rwlock_t *restrict rwlock, const struct timespec *restrict abstime);
WLIBC_API int wlibc_rwlock_wrlock(rwlock_t *rwlock);
WLIBC_API int wlibc_rwlock_trywrlock(rwlock_t *rwlock);
WLIBC_API int wlibc_rwlock_timedwrlock(rwlock_t *restrict rwlock, const struct timespec *restrict abstime);
WLIBC_API int wlibc_rwlock_unlock(rwlock_t *rwlock);
WLIBC_API int wlibc_rwlockattr_init(rwlock_attr_t *attributes);
WLIBC_API int wlibc_rwlockattr_getpshared(const rwlock_attr_t *restrict attributes, int *restrict pshared);
WLIBC_API int wlibc_rwlockattr_setpshared(rwlock_attr_t *attributes, int pshared);

// Thread specific storage functions.
WLIBC_API int wlibc_tss_create(key_t *index, dtor_t destructor);
WLIBC_API void *wlibc_tss_get(key_t index);
WLIBC_API int wlibc_tss_set(key_t index, const void *data);
WLIBC_API int wlibc_tss_delete(key_t index);

_WLIBC_END_DECLS

#endif
