#[[
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT
]]

set(sys_resource_SOURCES
rlimit.c)

add_library(sys_resource OBJECT ${sys_resource_SOURCES})
install(FILES ${CMAKE_SOURCE_DIR}/include/sys/resource.h DESTINATION include/sys)
