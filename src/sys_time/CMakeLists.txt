#[[
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT
]]

set(sys_time_SOURCES
gettimeofday.c)

add_library(sys_time OBJECT ${sys_time_SOURCES})

install(FILES ${CMAKE_SOURCE_DIR}/include/sys/time.h DESTINATION include/sys)
