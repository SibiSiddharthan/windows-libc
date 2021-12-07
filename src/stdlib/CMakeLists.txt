#[[
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

wlibc_module(
MODULE stdlib

SOURCES
mkstemp.c
setenv.c
unsetenv.c

HEADERS
stdlib-ext.h
)
