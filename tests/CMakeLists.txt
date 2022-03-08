#[[
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

include(WLIBCTests)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if(ENABLE_DLFCN)
	add_subdirectory(dlfcn)
endif()

if(ENABLE_POSIX_IO)
	add_subdirectory(fcntl)
	add_subdirectory(dirent)
	add_subdirectory(stdio)
	add_subdirectory(sys/file)
	add_subdirectory(sys/stat)
	add_subdirectory(unistd)
endif()

if(ENABLE_STDLIB_EXT)
	add_subdirectory(stdlib)
endif()

if(ENABLE_LANGINFO)
	add_subdirectory(langinfo)
endif()

if(ENABLE_SIGNALS)
	add_subdirectory(signal)
endif()

if(ENABLE_PROCESS)
	add_subdirectory(sys/wait)
endif()

if(ENABLE_RANDOM)
	add_subdirectory(sys/random)
endif()

if(ENABLE_ERROR_LOGGING)
	add_subdirectory(error)
endif()

if(ENABLE_EXTENDED_ATTRIBUTES)
	add_subdirectory(sys/xattr)
endif()

add_subdirectory(sys/utsname)

if(ENABLE_ACCOUNTS)
	add_subdirectory(grp)
	add_subdirectory(pwd)
endif()

if(ENABLE_MMAP)
	add_subdirectory(sys/mman)
endif()

if(ENABLE_THREADS)
	add_subdirectory(pthread)
	add_subdirectory(threads)
endif()

if(ENABLE_SPAWN)
	add_subdirectory(spawn)
endif()

