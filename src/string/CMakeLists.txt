#[[
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

set(string_SOURCES
ffs.c
#memchr.c
#mempcpy.c
#stpcpy.c
)

add_library(string OBJECT ${string_SOURCES})
#install(FILES ${CMAKE_SOURCE_DIR}/include/string-ext.h DESTINATION include)
install(FILES ${CMAKE_SOURCE_DIR}/include/strings.h DESTINATION include)
