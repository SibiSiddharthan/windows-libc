#[[
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

wlibc_module(
MODULE sys.statvfs

SOURCES
statvfs.c

HEADERS
sys/statvfs.h
sys/fstypes.h
)
