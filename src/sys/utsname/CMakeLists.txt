#[[
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

set(sys_utsname_SOURCES
uname.c)

add_library(sys_utsname OBJECT ${sys_utsname_SOURCES})
install(FILES ${CMAKE_SOURCE_DIR}/include/sys/utsname.h DESTINATION include/sys)
