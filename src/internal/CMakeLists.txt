#[[
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

set(internal_SOURCES
convert.c
error.c
misc.c
path.c
registry.c
security.c)

add_library(internal OBJECT ${internal_SOURCES})
add_library(wmain OBJECT wmain.c)

if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
	target_link_options(wmain PUBLIC -Wl,-entry:wmainCRTStartup)
elseif(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
	target_link_options(wmain PUBLIC /ENTRY:wmainCRTStartup /SUBSYSTEM:CONSOLE)
endif()

if(ENABLE_POSIX_IO)
	target_compile_definitions(wmain PRIVATE WLIBC_IO)
endif()
if(ENABLE_POSIX_SIGNALS)
	target_compile_definitions(wmain PRIVATE WLIBC_SIGNALS)
endif()
if(ENABLE_SPAWN)
	target_compile_definitions(wmain PRIVATE WLIBC_SPAWN)
endif()
if(ENABLE_THREADS)
	target_compile_definitions(wmain PRIVATE WLIBC_THREADS)
endif()
if(ENABLE_ACLS)
	target_compile_definitions(wmain PRIVATE WLIBC_ACLS)
endif()
if(ENABLE_TIMERS)
	target_compile_definitions(wmain PRIVATE WLIBC_TIMERS)
endif()
if(ENABLE_EXTENDED_ERRNO)
	target_compile_definitions(wmain PRIVATE WLIBC_ERRNO)
endif()
