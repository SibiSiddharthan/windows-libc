#[[
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

set(sys_resource_SOURCES
rlimit.c)

add_library(sys_resource OBJECT ${sys_resource_SOURCES})
install(FILES ${CMAKE_SOURCE_DIR}/include/sys/resource.h DESTINATION include/sys)
