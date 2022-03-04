/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_PTHREAD_H
#define WLIBC_PTHREAD_H

#include <wlibc.h>
#include <thread.h>

_WLIBC_BEGIN_DECLS

typedef thread_t pthread_t;
typedef thread_attr_t pthread_attr_t;
typedef void *(*pthread_start_t)(void *);

typedef once_t pthread_once_t;

typedef mutex_t pthread_mutex_t;
typedef mutex_attr_t pthread_mutexattr_t;

typedef cond_t pthread_cond_t;
typedef cond_attr_t pthread_condattr_t;

typedef barrier_t pthread_barrier_t;
typedef barrier_attr_t pthread_barrierattr_t;

typedef rwlock_t pthread_rwlock_t;
typedef rwlock_attr_t pthread_rwlockattr_t;

typedef key_t pthread_key_t;

#define PTHREAD_ONCE_INIT WLIBC_THREAD_ONCE_INIT

#define PTHREAD_CREATE_JOINABLE WLIBC_THREAD_JOINABLE // Joinable thread.
#define PTHREAD_CREATE_DETACHED WLIBC_THREAD_DETACHED // Detached thread.

#define PTHREAD_PROCESS_PRIVATE WLIBC_PROCESS_PRIVATE // Private to a process.
#define PTHREAD_PROCESS_SHARED  WLIBC_PROCESS_SHARED  // Shareabled across processes.

#define PTHREAD_MUTEX_NORMAL     WLIBC_MUTEX_TIMED
#define PTHREAD_MUTEX_DEFAULT    PTHREAD_MUTEX_NORMAL
#define PTHREAD_MUTEX_RECURSIVE  (WLIBC_MUTEX_RECURSIVE | WLIBC_MUTEX_TIMED)
#define PTHREAD_MUTEX_ERRORCHECK (WLIBC_MUTEX_NORMAL | WLIBC_MUTEX_TIMED)

// Thread functions.
WLIBC_INLINE int pthread_create(pthread_t *thread, pthread_attr_t *attributes, pthread_start_t routine, void *arg)
{
	return wlibc_thread_create(thread, attributes, routine, arg);
}

WLIBC_INLINE void pthread_exit(void *retval)
{
	wlibc_thread_exit_p(retval);
}

WLIBC_INLINE int pthread_detach(pthread_t thread)
{
	return wlibc_thread_detach(thread);
}

WLIBC_INLINE int pthread_join(pthread_t thread, void **result)
{
	return wlibc_thread_join(thread, result);
}

WLIBC_INLINE int pthread_equal(pthread_t thread_a, pthread_t thread_b)
{
	return wlibc_thread_equal(thread_a, thread_b);
}

WLIBC_INLINE pthread_t pthread_self(void)
{
	return wlibc_thread_self();
}

WLIBC_INLINE int pthread_yield(void)
{
	return wlibc_thread_yield();
}

WLIBC_INLINE int pthread_tryjoin(pthread_t thread, void **result)
{
	return wlibc_thread_tryjoin(thread, result);
}

WLIBC_INLINE int pthread_timedjoin(pthread_t thread, void **result, const struct timespec *abstime)
{
	return wlibc_thread_timedjoin(thread, result, abstime);
}

#define pthread_tryjoin_np   pthread_tryjoin
#define pthread_timedjoin_np pthread_timedjoin

// Attributes
WLIBC_INLINE int pthread_attr_init(pthread_attr_t *attributes)
{
	return wlibc_threadattr_init(attributes);
}

WLIBC_INLINE int pthread_attr_destroy(pthread_attr_t *attributes)
{
	// nop
	return 0;
}

WLIBC_INLINE int pthread_attr_getdetachstate(const pthread_attr_t *attributes, int *detachstate)
{
	return wlibc_threadattr_getdetachstate(attributes, detachstate);
}

WLIBC_INLINE int pthread_attr_setdetachstate(pthread_attr_t *attributes, int detachstate)
{
	return wlibc_threadattr_setdetachstate(attributes, detachstate);
}

WLIBC_INLINE int pthread_attr_getstacksize(const pthread_attr_t *restrict attributes, size_t *restrict stacksize)
{
	return wlibc_threadattr_getstacksize(attributes, stacksize);
}

WLIBC_INLINE int pthread_attr_setstacksize(pthread_attr_t *attributes, size_t stacksize)
{
	return wlibc_threadattr_setstacksize(attributes, stacksize);
}

// One time initialization.
WLIBC_INLINE int pthread_once(pthread_once_t *control, void (*init)(void))
{
	return wlibc_thread_once(control, init);
}

// Mutex functions.
WLIBC_INLINE int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attributes)
{
	return wlibc_mutex_init(mutex, attributes);
}

WLIBC_INLINE int pthread_mutex_destroy(pthread_mutex_t *mutex)
{
	return wlibc_mutex_destroy(mutex);
}

WLIBC_INLINE int pthread_mutex_trylock(pthread_mutex_t *mutex)
{
	return wlibc_mutex_trylock(mutex);
}

WLIBC_INLINE int pthread_mutex_lock(pthread_mutex_t *mutex)
{
	return wlibc_mutex_lock(mutex);
}

WLIBC_INLINE int pthread_mutex_timedlock(pthread_mutex_t *restrict mutex, const struct timespec *restrict abstime)
{
	return wlibc_mutex_timedlock(mutex, abstime);
}

WLIBC_INLINE int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
	return wlibc_mutex_unlock(mutex);
}

// Mutex attributes
WLIBC_INLINE int pthread_mutexattr_init(pthread_mutexattr_t *attributes)
{
	return wlibc_mutexattr_init(attributes);
}

WLIBC_INLINE int pthread_mutexattr_destroy(pthread_mutexattr_t *attributes)
{
	// nop
	return 0;
}

WLIBC_INLINE int pthread_mutexattr_getpshared(const pthread_mutexattr_t *restrict attributes, int *restrict pshared)
{
	return wlibc_mutexattr_getpshared(attributes, pshared);
}

WLIBC_INLINE int pthread_mutexattr_setpshared(pthread_mutexattr_t *attributes, int pshared)
{
	return wlibc_mutexattr_setpshared(attributes, pshared);
}

WLIBC_INLINE int pthread_mutexattr_gettype(const pthread_mutexattr_t *restrict attributes, int *restrict type)
{
	return wlibc_mutexattr_gettype(attributes, type);
}

WLIBC_INLINE int pthread_mutexattr_settype(pthread_mutexattr_t *attributes, int type)
{
	return wlibc_mutexattr_settype(attributes, type);
}

// Reader-Writer lock functions.
WLIBC_INLINE int pthread_rwlock_init(pthread_rwlock_t *restrict rwlock, const pthread_rwlockattr_t *restrict attributes)
{
	return wlibc_rwlock_init(rwlock, attributes);
}

WLIBC_INLINE int pthread_rwlock_destroy(pthread_rwlock_t *rwlock)
{
	return wlibc_rwlock_destroy(rwlock);
}

WLIBC_INLINE int pthread_rwlock_rdlock(pthread_rwlock_t *rwlock)
{
	return wlibc_rwlock_rdlock(rwlock);
}

WLIBC_INLINE int pthread_rwlock_tryrdlock(pthread_rwlock_t *rwlock)
{
	return wlibc_rwlock_tryrdlock(rwlock);
}

WLIBC_INLINE int pthread_rwlock_timedrdlock(pthread_rwlock_t *restrict rwlock, const struct timespec *restrict abstime)
{
	return wlibc_rwlock_timedrdlock(rwlock, abstime);
}

WLIBC_INLINE int pthread_rwlock_wrlock(pthread_rwlock_t *rwlock)
{
	return wlibc_rwlock_wrlock(rwlock);
}

WLIBC_INLINE int pthread_rwlock_trywrlock(pthread_rwlock_t *rwlock)
{
	return wlibc_rwlock_trywrlock(rwlock);
}

WLIBC_INLINE int pthread_rwlock_timedwrlock(pthread_rwlock_t *restrict rwlock, const struct timespec *restrict abstime)
{
	return wlibc_rwlock_timedwrlock(rwlock, abstime);
}

WLIBC_INLINE int pthread_rwlock_unlock(pthread_rwlock_t *rwlock)
{
	return wlibc_rwlock_unlock(rwlock);
}

// Reader-Writer attributes.
WLIBC_INLINE int pthread_rwlockattr_init(pthread_rwlockattr_t *attributes)
{
	return wlibc_rwlockattr_init(attributes);
}

WLIBC_INLINE int pthread_rwlockattr_destroy(pthread_rwlockattr_t *attributes)
{
	// nop
	return 0;
}

WLIBC_INLINE int pthread_rwlockattr_getpshared(const pthread_rwlockattr_t *restrict attributes, int *restrict pshared)
{
	return wlibc_rwlockattr_getpshared(attributes, pshared);
}

WLIBC_INLINE int pthread_rwlockattr_setpshared(pthread_rwlockattr_t *attributes, int pshared)
{
	return wlibc_rwlockattr_setpshared(attributes, pshared);
}

// Barrier functions.

#define PTHREAD_BARRIER_SERIAL_THREAD 1

WLIBC_INLINE int pthread_barrier_init(pthread_barrier_t *restrict barrier, const pthread_barrierattr_t *restrict attributes,
									  unsigned int count)
{
	return wlibc_barrier_init(barrier, attributes, count);
}

WLIBC_INLINE int pthread_barrier_destroy(pthread_barrier_t *barrier)
{
	return wlibc_barrier_destroy(barrier);
}

WLIBC_INLINE int pthread_barrier_wait(pthread_barrier_t *barrier)
{
	return wlibc_barrier_wait(barrier);
}

// Barrier attributes.
WLIBC_INLINE int pthread_barrierattr_init(pthread_barrierattr_t *attributes)
{
	return wlibc_barrierattr_init(attributes);
}

WLIBC_INLINE int pthread_barrierattr_destroy(pthread_barrierattr_t *attributes)
{
	// nop
	return 0;
}

WLIBC_INLINE int pthread_barrierattr_getpshared(const pthread_barrierattr_t *restrict attributes, int *restrict pshared)
{
	return wlibc_barrierattr_getpshared(attributes, pshared);
}

WLIBC_INLINE int pthread_barrierattr_setpshared(pthread_barrierattr_t *attributes, int pshared)
{
	return wlibc_barrierattr_setpshared(attributes, pshared);
}

// Condition variable functions.
WLIBC_INLINE int pthread_cond_init(pthread_cond_t *restrict cond, const pthread_condattr_t *restrict attributes)
{
	return wlibc_cond_init(cond, attributes);
}

WLIBC_INLINE int pthread_cond_destroy(pthread_cond_t *cond)
{
	return wlibc_cond_destroy(cond);
}

WLIBC_INLINE int pthread_cond_signal(pthread_cond_t *cond)
{
	return wlibc_cond_signal(cond);
}

WLIBC_INLINE int pthread_cond_broadcast(pthread_cond_t *cond)
{
	return wlibc_cond_broadcast(cond);
}

WLIBC_INLINE int pthread_cond_wait(pthread_cond_t *restrict cond, pthread_mutex_t *restrict mutex)
{
	return wlibc_cond_wait(cond, mutex);
}

WLIBC_INLINE int pthread_cond_timedwait(pthread_cond_t *restrict cond, pthread_mutex_t *restrict mutex,
										const struct timespec *restrict abstime)
{
	return wlibc_cond_timedwait(cond, mutex, abstime);
}

// Condition variable attributes
WLIBC_INLINE int pthread_condattr_init(pthread_condattr_t *attributes)
{
	return wlibc_condattr_init(attributes);
}

WLIBC_INLINE int pthread_condattr_destroy(pthread_condattr_t *attributes)
{
	// nop
	return 0;
}

WLIBC_INLINE int pthread_condattr_getpshared(const pthread_condattr_t *restrict attributes, int *restrict pshared)
{
	return wlibc_condattr_getpshared(attributes, pshared);
}

WLIBC_INLINE int pthread_condattr_setpshared(pthread_condattr_t *attributes, int pshared)
{
	return wlibc_condattr_setpshared(attributes, pshared);
}

// Thread specific storage functions.
WLIBC_API int pthread_key_create(pthread_key_t *key, void (*destructor)(void *))
{
	return wlibc_tss_create(key, destructor);
}

WLIBC_API int pthread_key_delete(pthread_key_t key)
{
	return wlibc_tss_delete(key);
}

WLIBC_API void *pthread_getspecific(pthread_key_t key)
{
	return wlibc_tss_get(key);
}

WLIBC_API int pthread_setspecific(pthread_key_t key, const void *data)
{
	return wlibc_tss_set(key, data);
}

_WLIBC_END_DECLS

#endif
