/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_THREADS_H
#define WLIBC_THREADS_H

#include <wlibc.h>
#include <thread.h>

_WLIBC_BEGIN_DECLS

typedef thread_t thrd_t;
typedef int (*thrd_start_t)(void *);
typedef once_t once_flag;
typedef mutex_t mtx_t;
typedef cond_t cnd_t;
typedef key_t tss_t;
typedef void (*tss_dtor_t)(void *);

#define ONCE_FLAG_INIT      WLIBC_THREAD_ONCE_INIT
#define TSS_DTOR_ITERATIONS WLIBC_DTOR_ITERATIONS

// Exit and error codes.
enum
{
	thrd_success = 0,
	thrd_busy = 1,
	thrd_error = 2,
	thrd_nomem = 3,
	thrd_timedout = 4
};

// Mutex types.
enum
{
	mtx_plain = WLIBC_MUTEX_NORMAL,
	mtx_recursive = WLIBC_MUTEX_RECURSIVE,
	mtx_timed = WLIBC_MUTEX_TIMED
};

// Thread functions.
WLIBC_INLINE int thrd_create(thrd_t *thread, thrd_start_t routine, void *arg)
{
	return wlibc_thread_create(thread, NULL, (thread_start_t)routine, arg);
}

WLIBC_INLINE int thrd_detach(thrd_t thread)
{
	return wlibc_thread_detach(thread);
}

WLIBC_INLINE int thrd_join(thrd_t thread, int *result)
{
	int status;
	void *real_result;

	status = wlibc_thread_join(thread, &real_result);
	if (result)
	{
		*result = (int)(intptr_t)real_result;
	}

	return status;
}

WLIBC_INLINE int thrd_equal(thrd_t thread_a, thrd_t thread_b)
{
	return wlibc_thread_equal(thread_a, thread_b);
}

WLIBC_INLINE thrd_t thrd_current(void)
{
	return wlibc_thread_self();
}

WLIBC_INLINE int thrd_sleep(const struct timespec *duration, struct timespec *remaining)
{
	return wlibc_thread_sleep(duration, remaining);
}

WLIBC_INLINE void thrd_yield(void)
{
	wlibc_thread_yield();
}

WLIBC_INLINE WLIBC_NORETURN void thrd_exit(int result)
{
	wlibc_thread_exit((void *)(intptr_t)result);
}

// One time initialization.
WLIBC_INLINE void call_once(once_flag *flag, void (*func)(void))
{
	wlibc_thread_once(flag, func);
}

// Mutex functions.
WLIBC_INLINE int mtx_init(mtx_t *mutex, int type)
{
	struct _wlibc_mutex_attr_t mutex_attr = {WLIBC_PROCESS_PRIVATE, type};
	return wlibc_mutex_init(mutex, &mutex_attr);
}

WLIBC_INLINE int mtx_lock(mtx_t *mutex)
{
	return wlibc_mutex_lock(mutex);
}

WLIBC_INLINE int mtx_timedlock(mtx_t *restrict mutex, const struct timespec *restrict abstime)
{
	return wlibc_mutex_timedlock(mutex, abstime);
}

WLIBC_INLINE int mtx_trylock(mtx_t *mutex)
{
	return wlibc_mutex_trylock(mutex);
}

WLIBC_INLINE int mtx_unlock(mtx_t *mutex)
{
	return wlibc_mutex_unlock(mutex);
}

WLIBC_INLINE void mtx_destroy(mtx_t *mutex)
{
	wlibc_mutex_destroy(mutex);
}

// Condition variable functions.
WLIBC_INLINE int cnd_init(cnd_t *cond)
{
	return wlibc_cond_init(cond, NULL);
}

WLIBC_INLINE int cnd_signal(cnd_t *cond)
{
	return wlibc_cond_signal(cond);
}

WLIBC_INLINE int cnd_broadcast(cnd_t *cond)
{
	return wlibc_cond_broadcast(cond);
}

WLIBC_INLINE int cnd_wait(cnd_t *cond, mtx_t *mutex)
{
	return wlibc_cond_wait(cond, mutex);
}

WLIBC_INLINE int cnd_timedwait(cnd_t *restrict cond, mtx_t *restrict mutex, const struct timespec *restrict abstime)
{
	return wlibc_cond_timedwait(cond, mutex, abstime);
}

WLIBC_INLINE void cnd_destroy(cnd_t *cond)
{
	wlibc_cond_destroy(cond);
}

// Thread specific storage functions.
WLIBC_INLINE int tss_create(tss_t *index, tss_dtor_t destructor)
{
	return wlibc_tss_create(index, destructor);
}

WLIBC_INLINE void *tss_get(tss_t index)
{
	return wlibc_tss_get(index);
}

WLIBC_INLINE int tss_set(tss_t index, void *data)
{
	return wlibc_tss_set(index, (void *)data);
}

WLIBC_INLINE void tss_delete(tss_t index)
{
	wlibc_tss_delete(index);
}

_WLIBC_END_DECLS

#endif
