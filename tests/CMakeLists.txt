#[[
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Function for adding tests
function(wlibc_add_tests ...)
	foreach(t ${ARGV})
		add_executable(test-${t} test-${t}.c)
		target_link_libraries(test-${t} wlibc)
		add_test(NAME test-${t} COMMAND test-${t} WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
	endforeach()
endfunction()


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
