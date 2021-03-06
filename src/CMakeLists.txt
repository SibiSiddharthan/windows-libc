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
	add_subdirectory(sys_stat)
	add_subdirectory(unistd)
	list(APPEND modules fcntl dirent stdio sys_stat unistd)
endif()

if(ENABLE_SYS_TIME)
	add_subdirectory(sys_time)
	list(APPEND modules sys_time)
endif()

if(ENABLE_SYS_RESOURCE)
	add_subdirectory(sys_resource)
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
	add_subdirectory(sys_wait)
	list(APPEND modules sys_wait)
endif()

add_subdirectory(internal)

add_library(wlibc)
target_link_libraries(wlibc ${modules} wlibc_internal)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	set_target_properties(wlibc PROPERTIES RUNTIME_OUTPUT_NAME "wlibcd" LIBRARY_OUTPUT_NAME "wlibcd" ARCHIVE_OUTPUT_NAME "wlibcd")
endif()

install(TARGETS wlibc LIBRARY DESTINATION lib)
install(FILES ${CMAKE_SOURCE_DIR}/include/wlibc-macros.h DESTINATION include)
install(FILES ${CMAKE_SOURCE_DIR}/include/sys/types.h DESTINATION include/sys)
