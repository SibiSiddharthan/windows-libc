#[[
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

wlibc_module(
MODULE sys.resource

SOURCES
priority.c
rlimit.c
rusage.c

HEADERS
sys/resource.h
)
