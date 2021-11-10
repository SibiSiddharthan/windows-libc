#[[
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

set(error_SOURCES
error.c)

add_library(error OBJECT ${error_SOURCES})
install(FILES ${CMAKE_SOURCE_DIR}/include/error.h ${CMAKE_SOURCE_DIR}/include/err.h DESTINATION include)
