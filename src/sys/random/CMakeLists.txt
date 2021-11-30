#[[
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

set(sys_random_SOURCES
random.c)

add_library(sys_random OBJECT ${sys_random_SOURCES})
install(FILES ${CMAKE_SOURCE_DIR}/include/sys/random.h DESTINATION include/sys)

if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
	target_compile_options(sys_random PRIVATE -mrdrnd -mrdseed) # clang needs these flags enabled to use the machine intrinsics
endif()
