#[[
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

wlibc_module(
MODULE thread

SOURCES
barrier.c
cond.c
internal.c
key.c
mutex.c
once.c
rwlock.c
thread.c

HEADERS
pthread.h
thread.h
threads.h
)
