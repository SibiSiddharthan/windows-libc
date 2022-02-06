#[[
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

wlibc_module(
MODULE sys.mman

SOURCES
mlock.c
mmap.c
mprotect.c
msync.c
munlock.c
munmap.c

HEADERS
sys/mman.h
)
