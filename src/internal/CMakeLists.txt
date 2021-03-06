#[[
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

set(internal_SOURCES
misc.c
wlibc_errors.c
wmain.c)

add_library(wlibc_internal OBJECT ${internal_SOURCES})
if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
	target_link_options(wlibc_internal PUBLIC -Wl,-entry:wmainCRTStartup)
elseif(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
	target_link_options(wlibc_internal PUBLIC /ENTRY:wmainCRTStartup /SUBSYSTEM:CONSOLE)
endif()

if(ENABLE_DLFCN)
	target_compile_definitions(wlibc_internal PRIVATE WLIBC_DLFCN)
endif()
if(ENABLE_POSIX_IO)
	target_compile_definitions(wlibc_internal PRIVATE WLIBC_POSIX_IO)
endif()
if(ENABLE_LANGINFO)
	target_compile_definitions(wlibc_internal PRIVATE WLIBC_LANGINFO)
endif()
if(ENABLE_PROCESS)
	target_compile_definitions(wlibc_internal PRIVATE WLIBC_PROCESS)
endif()
if(ENABLE_SIGNALS)
	target_compile_definitions(wlibc_internal PRIVATE WLIBC_SIGNALS)
endif()
