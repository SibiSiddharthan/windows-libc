## Build Instructions
``
cmake -DENABLE_<MOUDLE_NAME>=ON ..
``

## Available Modules
 * DLFCN
 * POSIX_IO
 * POSIX_SIGNALS
 * SYS_TIME
 * SYS_RESOURCE
 * STDLIB_EXT
 * STRINGS
 * LANGINFO
 * RANDOM
 * SPAWN
 * THREADS
 * MMAP
 * ACCOUNTS
 * EXTENDED_ATTRIBUTES
 * ACL
 * EXTENDED_ERRNO
 * ERROR_LOGGING

## Modules
 * DLFCN
	* Headers: dlfcn.h
	* Functions for loading functions from shared libraries.
 * POSIX_IO
	* Headers: dirent.h, fcntl.h, stdio.h, sys/file.h, sys/ioctl.h, sys/mount.h, sys/stat.h, sys/statfs.h, sys/statvfs.h, unistd.h
	* Functions for doing file and directory operations.
 * POSIX_SIGNALS
	* Headers: signal.h
	* Functions to emualate POSIX signal behavior on Windows.
 * SYS_TIME
	* Headers: sys/time.h
 * SYS_RESOURCE
	* Headers: sys/resource.h
	* Functions for measuring resource usage and restricting usage quotas.
 * STDLIB_EXT
	* Headers: stdlib-ext.h
	* POSIX stdlib functions not present in ISO C (eg. setenv, unsetenv, mktemp, mkstemp).
 * STRINGS
	* Headers: strings.h
 * LANGINGO
	* Headers: langinfo.h
 * RANDOM
	* Headers: sys/random.h
	* Functions for generating random data (getrandom, getentropy)
 * SPAWN
	* Headers: spawn.h, sys/wait.h
	* Functions for process management.
 * THREADS
	* Headers: pthread.h, threads.h
	* Functions for thread management.
 * MMAP
	* Headers: sys/mman.h
	* Functions for memory mapping files and managing virtual memory.
 * ACCOUNTS
	* Headers: grp.h, pwd.h
	* Functions for managing users and groups
 * EXTENDED_ATTRIBUTES
	* Headers: sys/xattr.h
	* Functions to manipulate extended attributes of files and directories.
 * ACL
	* Headers: sys/acl.h
	* Functions to manipulate the ACL(Access Control List) of an object.
 * EXTENDED_ERRNO
	* Headers: errno.h
	* More errno values and global variables for determining program name (program_invocation_name, program_invocation_short_name).
 * ERROR_LOGGING
	* Headers: error.h, err.h
	* Functions for logging errors, warnings.

