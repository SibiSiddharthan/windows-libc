#[[
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

include(WLIBCModules)
add_compile_definitions(BUILDING_WLIBC)

if(BUILD_SHARED_LIBS)
	add_compile_definitions(WLIBC_DLL)
endif()

if(ENABLE_DLFCN)
	wlibc_add_module(dlfcn)
endif()

if(ENABLE_POSIX_IO)
	wlibc_add_module(dirent fcntl poll stdio sys.file sys.ioctl sys.mount sys.stat sys.statfs sys.statvfs unistd)
endif()

if(ENABLE_SYS_TIME)
	wlibc_add_module(sys.time)
endif()

if(ENABLE_SYS_RESOURCE)
	wlibc_add_module(sys.resource)
endif()

if(ENABLE_STDLIB_EXT)
	wlibc_add_module(stdlib)
endif()

if(ENABLE_LANGINFO)
	wlibc_add_module(langinfo)
endif()

if(ENABLE_POSIX_SIGNALS)
	wlibc_add_module(signal)
endif()

if(ENABLE_STRINGS)
	wlibc_add_module(string)
endif()

if(ENABLE_RANDOM)
	wlibc_add_module(sys.random)
endif()

if(ENABLE_EXTENDED_ERRNO)
	wlibc_add_module(errno)
endif()

if(ENABLE_ERROR_LOGGING)
	wlibc_add_module(error)
endif()

if(ENABLE_EXTENDED_ATTRIBUTES)
	wlibc_add_module(sys.xattr)
endif()

if(ENABLE_ACCOUNTS)
	wlibc_add_module(grp pwd)
endif()

if(ENABLE_MMAP)
	wlibc_add_module(sys.mman)
endif()

if(ENABLE_THREADS)
	wlibc_add_module(thread)
endif()

if(ENABLE_SPAWN)
	wlibc_add_module(spawn sys.wait)
endif()

if(ENABLE_THREADS OR ENABLE_SPAWN)
	wlibc_add_module(sched)
endif()

if(ENABLE_ACLS)
	wlibc_add_module(sys.acl)
endif()

wlibc_add_module(sys.utsname)
wlibc_add_module(sys.times)

add_subdirectory(internal)

add_library(wlibc)
target_link_libraries(wlibc ${wlibc_enabled_modules} internal ntdll Netapi32)

if(BUILD_SHARED_LIBS)
	# TODO Merge wmain with the import library.
	set_target_properties(wlibc PROPERTIES RUNTIME_OUTPUT_NAME_DEBUG "wlibcd" LIBRARY_OUTPUT_NAME_DEBUG "wlibcd" ARCHIVE_OUTPUT_NAME_DEBUG "wlibcd")
	set_target_properties(wlibc PROPERTIES RUNTIME_OUTPUT_NAME_RELEASE "wlibc" LIBRARY_OUTPUT_NAME_RELEASE "wlibc" ARCHIVE_OUTPUT_NAME_RELEASE "wlibc")
else()
	target_link_libraries(wlibc wmain)
	set_target_properties(wlibc PROPERTIES RUNTIME_OUTPUT_NAME_DEBUG "wlibcds" LIBRARY_OUTPUT_NAME_DEBUG "wlibcds" ARCHIVE_OUTPUT_NAME_DEBUG "wlibcds")
	set_target_properties(wlibc PROPERTIES RUNTIME_OUTPUT_NAME_RELEASE "wlibcs" LIBRARY_OUTPUT_NAME_RELEASE "wlibcs" ARCHIVE_OUTPUT_NAME_RELEASE "wlibcs")
endif()

# Force the output directory to be root of the build directory for all configurations.
set_target_properties(wlibc PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
									   LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
									   ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

foreach(config ${CMAKE_CONFIGURATION_TYPES})
	string(TOUPPER ${config} config)
	set_target_properties(wlibc PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${config} ${CMAKE_BINARY_DIR}
										   LIBRARY_OUTPUT_DIRECTORY_${config} ${CMAKE_BINARY_DIR}
										   ARCHIVE_OUTPUT_DIRECTORY_${config} ${CMAKE_BINARY_DIR})
endforeach()

install(TARGETS wlibc LIBRARY DESTINATION lib)
install(FILES ${CMAKE_SOURCE_DIR}/include/wlibc.h DESTINATION include)
install(FILES ${CMAKE_SOURCE_DIR}/include/sys/types.h DESTINATION include/sys)
install(FILES ${CMAKE_SOURCE_DIR}/include/sys/param.h DESTINATION include/sys)

install(FILES ${CMAKE_SOURCE_DIR}/include/alloca.h DESTINATION include)
install(FILES ${CMAKE_SOURCE_DIR}/include/byteswap.h DESTINATION include)
install(FILES ${CMAKE_SOURCE_DIR}/include/io.h DESTINATION include)
install(FILES ${CMAKE_SOURCE_DIR}/include/limits.h DESTINATION include)
install(FILES ${CMAKE_SOURCE_DIR}/include/posix/limits.h DESTINATION include/posix)
install(FILES ${CMAKE_SOURCE_DIR}/include/stdint.h DESTINATION include)
install(FILES ${CMAKE_SOURCE_DIR}/include/sysexits.h DESTINATION include)
install(FILES ${CMAKE_SOURCE_DIR}/include/wchar.h DESTINATION include)
