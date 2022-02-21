/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_THREAD_H
#define WLIBC_THREAD_H

#include <wlibc.h>
#include <time.h>

_WLIBC_BEGIN_DECLS

typedef struct _wlibc_thread_t
{
	int handle;
	int id;
} thread_t;

typedef struct _wlibc_thread_attr_t
{
	int dummy;
} thread_attr_t;

typedef struct _wlibc_mutex_attr_t
{
	int dummy;
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
	int dummy;
} rwlock_attr_t;

typedef void *(*thread_start_t)(void *);

typedef union _wlibc_once_t {
	void *ptr;
} once_t;

typedef struct _wlibc_mutex_t
{
	unsigned int owner;
	void *handle;
} mutex_t;

typedef struct _wlibc_cond_t
{
	void *ptr;
} cond_t;

typedef struct _wlibc_barrier_t
{
	unsigned int reserved[8];
} barrier_t;

typedef struct _wlibc_rwlock_t
{
	void *ptr;
} rwlock_t;

typedef void (*dtor_t)(void *);
typedef struct _wlibc_key_t
{
	int index;
	void (*destructor)(void *);
} key_t;

#define WLIBC_THREAD_ONCE_INIT {0}
#define WLIBC_DTOR_ITERATIONS 2

#define WLIBC_PROCESS_PRIVATE 0 // Private to a process.
#define WLIBC_PROCESS_SHARED  1 // Shareabled across processes.

// Thread functions.
WLIBC_API int wlibc_thread_create(thread_t *thread, thread_attr_t *attributes, thread_start_t routine, void *arg);
WLIBC_API int wlibc_thread_detach(thread_t thread);
WLIBC_API int wlibc_thread_join(thread_t thread, void **result);
WLIBC_API int wlibc_thread_tryjoin(thread_t thread, void **result);
WLIBC_API int wlibc_thread_timedjoin(thread_t thread, void **result, const struct timespec *abstime);
WLIBC_API int wlibc_thread_equal(thread_t thread_a, thread_t thread_b);
WLIBC_API thread_t wlibc_thread_self(void);
WLIBC_API int wlibc_thread_sleep(const struct timespec *duration, struct timespec *remaining);
WLIBC_API int wlibc_thread_yield(void);
WLIBC_API void wlibc_thread_exit_p(void *retval);
WLIBC_API void wlibc_thread_exit_c11(int result);

// One time initialization.
WLIBC_API int wlibc_thread_once(once_t *control, void (*init)(void));

// Mutex functions.
WLIBC_API int wlibc_mutex_init(mutex_t *mutex, const mutex_attr_t *attributes);
WLIBC_API int wlibc_mutex_destroy(mutex_t *mutex);
WLIBC_API int wlibc_mutex_trylock(mutex_t *mutex);
WLIBC_API int wlibc_mutex_lock(mutex_t *mutex);
WLIBC_API int wlibc_mutex_timedlock(mutex_t *restrict mutex, const struct timespec *restrict abstime);
WLIBC_API int wlibc_mutex_unlock(mutex_t *mutex);

// Condition variable functions.
WLIBC_API int wlibc_cond_init(cond_t *restrict cond, const cond_attr_t *restrict attributes);
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
WLIBC_API int wlibc_rwlock_rdlock(rwlock_t *rwlock);
WLIBC_API int wlibc_rwlock_tryrdlock(rwlock_t *rwlock);
WLIBC_API int wlibc_rwlock_timedrdlock(rwlock_t *restrict rwlock, const struct timespec *restrict abstime);
WLIBC_API int wlibc_rwlock_wrlock(rwlock_t *rwlock);
WLIBC_API int wlibc_rwlock_trywrlock(rwlock_t *rwlock);
WLIBC_API int wlibc_rwlock_timedwrlock(rwlock_t *restrict rwlock, const struct timespec *restrict abstime);
WLIBC_API int wlibc_rwlock_unlock(rwlock_t *rwlock);

// Thread specific storage functions.
WLIBC_API int wlibc_tss_create(key_t *index, dtor_t destructor);
WLIBC_API void *wlibc_tss_get(key_t index);
WLIBC_API int wlibc_tss_set(key_t index, const void *data);
WLIBC_API int wlibc_tss_delete(key_t index);

_WLIBC_END_DECLS

#endif
