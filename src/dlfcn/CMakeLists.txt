#[[
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

set(dlfcn_SOURCES
dlclose.c
dlerror.c
dlopen.c
dlsym.c
dlfcn_internal.c)

add_library(dlfcn OBJECT ${dlfcn_SOURCES})

install(FILES ${CMAKE_SOURCE_DIR}/include/dlfcn.h DESTINATION include)
