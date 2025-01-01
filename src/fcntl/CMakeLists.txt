#[[
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

wlibc_module(
MODULE fcntl

SOURCES
fcntl.c
internal.c
open.c
osfhandle.c

HEADERS
fcntl.h
)
