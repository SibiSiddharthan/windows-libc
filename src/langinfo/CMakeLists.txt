#[[
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

set(langinfo_SOURCES
nl_langinfo.c
langinfo_internal.c)

add_library(langinfo OBJECT ${langinfo_SOURCES})

install(FILES ${CMAKE_SOURCE_DIR}/include/langinfo.h DESTINATION include)
