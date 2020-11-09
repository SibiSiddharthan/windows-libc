#[[
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT
]]

set(stdlib_SOURCES
setenv.c
unsetenv.c)

add_library(stdlib OBJECT ${stdlib_SOURCES})
install(FILES ${CMAKE_SOURCE_DIR}/include/stdlib-ext.h DESTINATION include)
