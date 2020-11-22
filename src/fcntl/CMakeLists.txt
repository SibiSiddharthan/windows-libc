#[[
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

set(fcntl_SOURCES
open.c
openat.c
creat.c
fcntl.c
fcntl_internal.c)

add_library(fcntl OBJECT ${fcntl_SOURCES})

install(FILES ${CMAKE_SOURCE_DIR}/include/fcntl.h DESTINATION include)
