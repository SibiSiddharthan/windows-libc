#[[
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

cmake_minimum_required(VERSION 3.15)
project(wlibc C)

if(NOT WIN32)
	message(FATAL_ERROR "Unsupported platform, only Windows is supported")
endif()

add_compile_definitions(UNICODE _UNICODE
						WIN32_LEAN_AND_MEAN UMDF_USING_NTSTATUS
						_CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_DEPRECATE)

if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
	add_compile_options(-fcolor-diagnostics -fansi-escape-codes -Wall -Wno-unused-variable -std=c11)
endif()

if(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
	add_compile_options(/wd4098 /wd4477 /std:c11)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include_directories(include)

option(BUILD_SHARED_LIBS "Build Shared Libraries" OFF)
option(ENABLE_DLFCN "Enable dlfcn module" ON)
option(ENABLE_POSIX_IO "Enable POSIX-like IO" ON)
option(ENABLE_SYS_TIME "Enable sys/time.h" ON)
option(ENABLE_SYS_RESOURCE "Enable sys/resource.h" ON)
option(ENABLE_STDLIB_EXT "Enable stdlib extensions" ON)
option(ENABLE_LANGINFO "Enable langinfo.h" ON)
option(ENABLE_SIGNALS "Enable POSIX like signals" ON)
option(ENABLE_PROCESS "Enable POSIX process management" ON)
option(ENABLE_BINARY_STD_STREAMS "Enable binary(instead of text) std streams" ON)
option(ENABLE_RANDOM "Enable random functions" ON)
option(ENABLE_ERROR_LOGGING "Enable error logging functions" ON)
option(ENABLE_EXTENDED_ATTRIBUTES "Enable NTFS extended attributes API" ON)
option(ENABLE_ACL_SUPPORT "Use ACLs for controlling permissions of files" ON)
option(ENABLE_STRINGS "Enable extra string functions" ON)
#option(ENABLE_SOCKETS "Enable SOCKETS API" ON)

add_subdirectory(src)

include(CTest)
if(BUILD_TESTING)
	add_subdirectory(tests)
endif()
