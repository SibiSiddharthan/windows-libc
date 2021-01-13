#[[
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

set(stdio_SOURCES
parse_mode.c
pipe-hooks.c
stdio-hooks.c
rename.c
renameat.c)

add_library(stdio OBJECT ${stdio_SOURCES})
install(FILES ${CMAKE_SOURCE_DIR}/include/stdio-ext.h DESTINATION include)
install(FILES ${CMAKE_SOURCE_DIR}/include/stdio-hooks.h DESTINATION include)
