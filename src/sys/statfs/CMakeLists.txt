#[[
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

wlibc_module(
MODULE sys.statfs

SOURCES
statfs.c

HEADERS
sys/statfs.h
sys/fstypes.h
)
