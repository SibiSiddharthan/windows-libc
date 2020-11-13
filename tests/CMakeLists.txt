#[[
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if(ENABLE_DLFCN)
	add_subdirectory(dlfcn)
endif()

if(ENABLE_POSIX_IO)
	add_subdirectory(fcntl)
	add_subdirectory(dirent)
	add_subdirectory(stdio)
	add_subdirectory(sys_stat)
	add_subdirectory(unistd)
endif()

if(ENABLE_STDLIB_EXT)
	add_subdirectory(stdlib)
	list(APPEND modules stdlib)
endif()
