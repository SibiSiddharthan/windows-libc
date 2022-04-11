#[[
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

wlibc_module(
MODULE sched

SOURCES
affinity.c
cpuset.c
param.c
scheduler.c
yield.c

HEADERS
sched.h
)
