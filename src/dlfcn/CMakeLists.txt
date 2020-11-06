#[[
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT
]]

set(dlfcn_SOURCES
dlclose.c
dlerror.c
dlopen.c
dlsym.c
dlfcn_internal.c)

add_library(dlfcn OBJECT ${dlfcn_SOURCES})

install(FILES ${CMAKE_SOURCE_DIR}/include/dlfcn.h DESTINATION include)
