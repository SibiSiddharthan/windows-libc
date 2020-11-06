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

add_subdirectory(internal)

add_library(wlibc)
target_link_libraries(wlibc ${modules} wlibc_internal)

install(TARGETS wlibc LIBRARY DESTINATION lib)
install(FILES ${CMAKE_SOURCE_DIR}/include/wlibc-macros.h DESTINATION include)
install(FILES ${CMAKE_SOURCE_DIR}/include/sys/types.h DESTINATION include/sys)
