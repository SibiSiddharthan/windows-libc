#[[
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

set(sys_file_SOURCES
flock.c)

add_library(sys_file OBJECT ${sys_file_SOURCES})
install(FILES ${CMAKE_SOURCE_DIR}/include/sys/file.h DESTINATION include/sys)
