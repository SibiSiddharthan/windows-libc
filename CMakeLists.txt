#[[
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

cmake_minimum_required(VERSION 3.15)
project(wlibc C)

if(NOT WIN32)
	message(FATAL_ERROR "Unsupported platform, only Windows is supported")
endif()

add_compile_definitions(UNICODE _UNICODE
						WIN32_LEAN_AND_MEAN
						_CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_DEPRECATE)

if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
	add_compile_options(-fcolor-diagnostics -fansi-escape-codes -Wall -Wno-unused-variable)
endif()

include_directories(include include/internal)

option(BUILD_SHARED_LIBS "Build Shared Libraries" OFF)
option(ENABLE_DLFCN "Enable dlfcn module" ON)
option(ENABLE_POSIX_IO "Enable POSIX-like IO" ON)
option(ENABLE_SYS_TIME "Enable sys/time.h" ON)
option(ENABLE_SYS_RESOURCE "Enable sys/resource.h" ON)
option(ENABLE_STDLIB_EXT "Enable stdlib extensions" ON)

add_subdirectory(src)

include(CTest)
if(BUILD_TESTING)
	add_subdirectory(tests)
endif()
