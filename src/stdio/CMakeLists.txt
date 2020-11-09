#[[
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT
]]

set(stdio_SOURCES
stdio-hooks.c
rename.c
renameat.c)

add_library(stdio OBJECT ${stdio_SOURCES})
install(FILES ${CMAKE_SOURCE_DIR}/include/stdio-ext.h DESTINATION include)
install(FILES ${CMAKE_SOURCE_DIR}/include/stdio-hooks.h DESTINATION include)
