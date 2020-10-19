set(internal_SOURCES
misc.c
wlibc_errors.c
wmain.c)

add_library(wlibc_internal OBJECT ${internal_SOURCES})
target_link_options(wlibc_internal PUBLIC -Wl,-entry:wmainCRTStartup)

if(ENABLE_DLFCN)
	target_compile_definitions(wlibc_internal PRIVATE WLIBC_DLFCN)
endif()
if(ENABLE_POSIX_IO)
	target_compile_definitions(wlibc_internal PRIVATE WLIBC_POSIX_IO)
endif()
