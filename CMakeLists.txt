#[[
   Copyright (c) 2020-2025 Sibi Siddharthan

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
	add_compile_options(-fcolor-diagnostics -fansi-escape-codes
						-Wall -Wextra
						-Wno-cast-function-type-mismatch
						-std=c11)
endif()

if(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
	add_compile_options(/W4
						/wd4146 # Unary minus
						/wd4714 # No inline in debug build
						/wd5072 # ASAN enabled with O2
						/std:c11
						/utf-8)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include_directories(include)

option(BUILD_SHARED_LIBS "Build Shared Libraries" OFF)
option(ENABLE_ASAN "Use address sanitizer" OFF)

# List of modules
option(ENABLE_DLFCN "Enable dlfcn module" ON)
option(ENABLE_GETOPT "Enable getopt module" ON)
option(ENABLE_POSIX_IO "Enable POSIX like IO" ON)
option(ENABLE_POSIX_SIGNALS "Enable POSIX like signals" ON)
option(ENABLE_SYS_RESOURCE "Enable sys/resource.h" ON)
option(ENABLE_STDLIB_EXT "Enable stdlib extensions" ON)
option(ENABLE_LANGINFO "Enable langinfo.h" ON)
option(ENABLE_RANDOM "Enable random functions" ON)
option(ENABLE_EXTENDED_ERRNO "Enable extended errno functionality" ON)
option(ENABLE_ERROR_LOGGING "Enable error logging functions" ON)
option(ENABLE_EXTENDED_ATTRIBUTES "Enable NTFS extended attributes API" ON)
option(ENABLE_ACLS "Use ACLs for controlling permissions of files" ON)
option(ENABLE_STRING_EXT "Enable string extensions" ON)
option(ENABLE_ACCOUNTS "Enable user(pwd) and group(grp) functions" ON)
option(ENABLE_MMAP "Enable POSIX mmap API" ON)
option(ENABLE_THREADS "Enable pthreads and C11 threads API" ON)
option(ENABLE_SPAWN "Enable POSIX spawn API" ON)
option(ENABLE_TIMERS "Enable POSIX timers" ON)
option(ENABLE_TERMIOS "Enable termios module" ON)
option(ENABLE_WCHAR_EXT "Enable wchar extensions" ON)

# Only enable the address sanitizer with MSVC.
if(ENABLE_ASAN)
	if(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
		add_compile_definitions(WLIBC_ASAN_BUILD)
		add_compile_options(/fsanitize=address)
		add_link_options(/incremental:no)
	endif()
endif()

add_subdirectory(src)

include(CTest)
if(BUILD_TESTING)
	add_subdirectory(tests)
endif()
