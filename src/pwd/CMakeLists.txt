#[[
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

wlibc_module(
MODULE pwd

SOURCES
pwent.c
pwnam.c
pwuid.c

HEADERS
pwd.h
)
