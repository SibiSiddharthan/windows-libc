## Build Instructions
``
cmake -D<MOUDLE_NAME>=ON ..
``

## Available Modules
 * DLFCN
 * POSIX_IO
 * SYS_TIME
 * SYS_RESOURCE
 * STDLIB_EXT

## Modules
 * DLFCN
	* Headers: dlfcn.h
	* Functions for loading functions from shared libraries
 * POSIX_IO
	* Headers: dirent.h, fcntl.h, stdio-ext.h, stdio-hooks.h, sys/stat.h, unistd.h
	* Functions for doing file and directory operations
 * SYS_TIME
	* Headers: sys/time.h
 * SYS_RESOURCE
	* Headers: sys/resource.h
 * STDLIB_EXT
	* Headers: stdlib-ext.h
	* POSIX stdlib functions (eg. setenv, unsetenv)
