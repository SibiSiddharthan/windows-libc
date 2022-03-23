#[[
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

set(internal_SOURCES
error.c
misc.c
path.c
registry.c
security.c
wmain.c)

add_library(wlibc_internal OBJECT ${internal_SOURCES})
if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
	target_link_options(wlibc_internal PUBLIC -Wl,-entry:wmainCRTStartup)
elseif(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
	target_link_options(wlibc_internal PUBLIC /ENTRY:wmainCRTStartup /SUBSYSTEM:CONSOLE)
endif()

if(ENABLE_POSIX_IO)
	target_compile_definitions(wlibc_internal PRIVATE WLIBC_POSIX_IO)
endif()
if(ENABLE_PROCESS)
	target_compile_definitions(wlibc_internal PRIVATE WLIBC_PROCESS)
endif()
if(ENABLE_SIGNALS)
	target_compile_definitions(wlibc_internal PRIVATE WLIBC_SIGNALS)
endif()
if(ENABLE_THREADS)
	target_compile_definitions(wlibc_internal PRIVATE WLIBC_THREADS)
endif()
if(ENABLE_ACL_SUPPORT)
	target_compile_definitions(wlibc_internal PRIVATE WLIBC_ACLS)
endif()
