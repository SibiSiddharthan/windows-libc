#[[
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

wlibc_module(
MODULE sys.time

SOURCES
clock.c
gettimeofday.c
internal.c
itimer.c
timer.c
utimes.c

HEADERS
sys/time.h
)
