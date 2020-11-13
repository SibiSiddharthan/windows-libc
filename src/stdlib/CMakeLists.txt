#[[
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

set(stdlib_SOURCES
setenv.c
unsetenv.c)

add_library(stdlib OBJECT ${stdlib_SOURCES})
install(FILES ${CMAKE_SOURCE_DIR}/include/stdlib-ext.h DESTINATION include)
