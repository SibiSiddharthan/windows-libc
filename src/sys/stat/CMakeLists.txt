#[[
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

wlibc_module(
MODULE sys.stat

SOURCES
chflags.c
chmod.c
mkdir.c
stat.c
statx.c
utimens.c

HEADERS
sys/stat.h
)
