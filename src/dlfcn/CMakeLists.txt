#[[
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

wlibc_module(
MODULE dlfcn

SOURCES
dlclose.c
dlerror.c
dlopen.c
dlsym.c

HEADERS
dlfcn.h
)
