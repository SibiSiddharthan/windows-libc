#[[
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

set(sys_wait_SOURCES
process_internal.c
wait.c
waitpid.c)

add_library(sys_wait OBJECT ${sys_wait_SOURCES})

install(FILES ${CMAKE_SOURCE_DIR}/include/sys/wait.h DESTINATION include/sys)
