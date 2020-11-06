set(dlfcn_SOURCES
dlclose.c
dlerror.c
dlopen.c
dlsym.c
dlfcn_internal.c)

add_library(dlfcn OBJECT ${dlfcn_SOURCES})

install(FILES ${CMAKE_SOURCE_DIR}/include/dlfcn.h DESTINATION include)
