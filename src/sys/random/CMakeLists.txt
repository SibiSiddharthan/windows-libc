#[[
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

wlibc_module(
MODULE sys.random

SOURCES
random.c

HEADERS
sys/random.h
)

if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
	target_compile_options(sys.random PRIVATE -mrdrnd -mrdseed) # clang needs these flags enabled to use the machine intrinsics
endif()
