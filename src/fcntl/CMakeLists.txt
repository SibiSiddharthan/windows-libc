#[[
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT
]]

set(fcntl_SOURCES
open.c
openat.c
creat.c
fcntl_internal.c)

add_library(fcntl OBJECT ${fcntl_SOURCES})

install(FILES ${CMAKE_SOURCE_DIR}/include/fcntl.h DESTINATION include)
