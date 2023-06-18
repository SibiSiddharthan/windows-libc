#[[
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

wlibc_module(
MODULE string

SOURCES
ffs.c
#memchr.c
#mempcpy.c
#stpcpy.c

HEADERS
#string-ext.h
strings.h
)
