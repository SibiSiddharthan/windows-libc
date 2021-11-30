#[[
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

set(sys_time_SOURCES
gettimeofday.c)

add_library(sys_time OBJECT ${sys_time_SOURCES})

install(FILES ${CMAKE_SOURCE_DIR}/include/sys/time.h DESTINATION include/sys)
