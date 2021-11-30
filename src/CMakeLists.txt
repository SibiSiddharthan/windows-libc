#[[
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

add_compile_definitions(BUILDING_WLIBC)

if(BUILD_SHARED_LIBS)
	add_compile_definitions(WLIBC_DLL)
endif()

if(ENABLE_DLFCN)
	add_subdirectory(dlfcn)
	list(APPEND modules dlfcn)
endif()

if(ENABLE_POSIX_IO)
	add_subdirectory(fcntl)
	add_subdirectory(dirent)
	add_subdirectory(stdio)
	add_subdirectory(sys/file)
	add_subdirectory(sys/stat)
	add_subdirectory(unistd)
	list(APPEND modules fcntl dirent stdio sys_file sys_stat unistd)
endif()

if(ENABLE_SYS_TIME)
	add_subdirectory(sys/time)
	list(APPEND modules sys_time)
endif()

if(ENABLE_SYS_RESOURCE)
	add_subdirectory(sys/resource)
	list(APPEND modules sys_resource)
endif()

if(ENABLE_STDLIB_EXT)
	add_subdirectory(stdlib)
	list(APPEND modules stdlib)
endif()

if(ENABLE_LANGINFO)
	add_subdirectory(langinfo)
	list(APPEND modules langinfo)
endif()

if(ENABLE_SIGNALS)
	add_subdirectory(signal)
	list(APPEND modules signal)
endif()

if(ENABLE_PROCESS)
	add_subdirectory(sys/wait)
	list(APPEND modules sys_wait)
endif()

if(ENABLE_STRINGS)
	add_subdirectory(string)
	list(APPEND modules string)
endif()

if(ENABLE_RANDOM)
	add_subdirectory(sys/random)
	list(APPEND modules sys_random)
endif()

if(ENABLE_ERROR_LOGGING)
	add_subdirectory(error)
	list(APPEND modules error)
endif()

if(ENABLE_EXTENDED_ATTRIBUTES)
	add_subdirectory(sys/xattr)
	list(APPEND modules sys_xattr)
endif()

add_subdirectory(internal)

add_library(wlibc)
target_link_libraries(wlibc ${modules} wlibc_internal ntdll)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	set_target_properties(wlibc PROPERTIES RUNTIME_OUTPUT_NAME "wlibcd" LIBRARY_OUTPUT_NAME "wlibcd" ARCHIVE_OUTPUT_NAME "wlibcd")
endif()

install(TARGETS wlibc LIBRARY DESTINATION lib)
install(FILES ${CMAKE_SOURCE_DIR}/include/wlibc-macros.h DESTINATION include)
install(FILES ${CMAKE_SOURCE_DIR}/include/sys/types.h DESTINATION include/sys)
install(FILES ${CMAKE_SOURCE_DIR}/include/wchar.h DESTINATION include)

install(FILES ${CMAKE_SOURCE_DIR}/include/io.h DESTINATION include)
install(FILES ${CMAKE_SOURCE_DIR}/include/alloca.h DESTINATION include)
