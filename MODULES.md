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
 * GETOPT
	* Headers: getopt.h
	* Funtions for handling command line arguments.
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


## Headers and Functions
 * dirent.h
	* Functions
		* Implemented
			* opendir, fdopendir, closedir, dirfd
			* readdir, readdir_r, rewinddir, seekdir, telldir
			* scandir, scandirat
			* alphasort
	* Notes
		* `struct dirent` has an extra member `d_namelen` to denote the length of the string in `d_name`.
 * dlfcn.h
	* Functions
		* dlopen, dlclose, dlsym, dlerror
	* Notes
		* The flags passed to `dlopen` (eg. RTLD_LAZY, RTLD_NOW) have no additional use.
 * err.h
	* Functions
		* err, errx, verr, verrx
		* warn, warnx, vwarn, vwarnx
 * error.h
	* Functions
		* error, verror
		* error_at_line, verror_at_line
 * fcntl.h
	* Functions
		* Implemented
			* open, openat, creat
			* fcntl
		* Unsupported
			* posix_fadvise, posix_fallocate
	* Notes
		* Extra flags are provided for `open` and `openat` to match the `CreateFile` API. These are `O_READONLY`, `O_HIDDEN`, `O_SYSTEM`, `O_ARCHIVE`, `O_ENCRYPTED`.
		* Supported fcntl operations are `F_DUPFD`, `F_DUPFD_CLOEXEC`, `F_GETFD`, `F_SETFD`, `F_GETFL`, `F_SETFL`.
 * getopt.h
	* Functions
		* getopt, getopt_long
 * grp.h
	* Functions
		* getgrent, getgrnam, getgrgid, endgrent, setgrent, 
		* getgrent_r, getgrnam_r, getgrgid_r
	* Notes
		* The groups returned by these functions are taken from the `NT AUTHORITY`, `BUILTIN` and local pc domains. 
 * pwd.h
	* Functions
		* getpwent, getpwnam, getpwuid, endpwent, setpwent, 
		* getpwent_r, getpwnam_r, getpwuid_r
 * poll.h
	* Functions
		* poll, ppoll
	* Notes
		* Polling for out of band data on sockets is not implemented yet.
		* Polling terminal state changes is not implemented yet.
 * sched.h
	* Functions
		* sched_getparam, sched_setparam
		* sched_getscheduler, sched_setscheduler
		* sched_getaffinity, sched_setaffinity
		* sched_get_priority_max, sched_get_priority_min
		* sched_yield
		* CPU_SET functions
	* Notes
		* The scheduling alogrithms (eg. `SCHED_IDLE`, `SCHED_RR`) point to priority classes (eg. `PROCESS_PRIORITY_CLASS_IDLE`, `PROCESS_PRIORITY_CLASS_NORMAL`)
		* The scheduling paramter goes from -2 to +2.
		* Setting core affinity is untested on multi-socket systems.
 * signal.h
	* Functions
		* Implemented
			* signal, raise
			* sigaction, sigprocmask
			* sigset functions
		* Unimplemented
			* sigwait, sigreturn
			* TBD
	* Notes
		* Per thread signal masks are implemented.
		* When sending a signal (except `SIGSTOP`, `SIGCONT`) to another process the said process is terminated with exit code `128 + SIGNUM`.
		* Sending `SIGSTOP` to a process suspends it. `SIGCONT` resumes it.
 * strings.h
	* Functions
		* bcmp, bcopy, bzero
		* ffs, ffsl, ffsll
		* index, rindex
		* strcasecmp, strncasecmp
 * spawn.h
	* Functions
		* posix_spawn, posix_spawnp
		* posix_spawn_actions_(addopen, addclose, adddup2, addchdir, addfchdir)
		* posix_spawn_attribues (sigmask, sigdefault, schedpolicy, schedparam, pgroup)
	* Notes
		* Windows argv mangling is implemented.
		* Unix shebang exec is implemented.
		* Setting process group is unimplemented.
		* Inheritance of signal mask is unimplemented.
 * threads.h
	* Functions
		* call_once
		* thrd_create, thrd_detach, thrd_join, thrd_equal, thrd_current, thrd_sleep, thrd_yield, thrd_exit
		* mtx_init, mtx_lock, mtx_timedlock, mtx_trylock, mtx_unlock, mtx_destroy.
		* cnd_init, cnd_signal, cnd_broadcast, cnd_wait, cnd_timedwait, cnd_destroy.
		* tss_create, tss_get, tss_set, tss_delete
 * pthreads.h
	* Functions
		* pthread_once
		* pthread_create, pthread_detach, pthread_join, pthread_equal, pthread_self, pthread_threadid, pthread_yield, pthread_tryjoin, pthread_timedjoin.
		* pthread_cancel, pthread_testcancel, pthread_setcancelstate, pthread_setcanceltype.
		* pthread_cleanup_push, pthread_cleanup_pop
		* pthread_resume, pthread_suspend
		* Thread attributes (detachstate, suspendstate, stacksize, scope, scheduling, affinity)
		* pthread_getaffinity, pthread_setaffinity, pthread_getname, pthread_setname
		* pthread_getschedparam, pthread_setschedparam, pthread_getschedprio, pthread_setschedprio
		* pthread_getconcurrency, pthread_setconcurrency
		* pthread_kill, pthread_sigmask
		* pthread_mutex_init, pthread_mutex_destroy, pthread_mutex_trylock, pthread_mutex_lock, pthread_mutex_timedlock, pthread_mutex_unlock
		* Mutex attributes (pshared, type)
		* pthread_rwlock_init, pthread_rwlock_destroy
		* pthread_rwlock_rdlock, pthread_rwlock_tryrdlock, pthread_rwlock_timedrdlock
		* pthread_rwlock_wrlock, pthread_rwlock_trywrlock, pthread_rwlock_timedwrlock
		* pthread_rwlock_unlock
		* Reader-Writer lock attributes (pshared)
		* pthread_barrier_init, pthread_barrier_destroy, pthread_barrier_wait
		* Barrier attributes (pshared)
		* pthread_cond_init, pthread_cond_destroy, pthread_cond_signal, pthread_cond_broadcast, pthread_cond_wait, pthread_cond_timedwait
		* Condition variable attributes (pshared)
		* pthread_key_create, pthread_key_delete, pthread_getspecific, pthread_setspecific.
	* Notes (Also applies to threads.h)
		* Functions for suspending and resuming a thread are added.
		* A thread can be created in a suspended state also.
		* Thread affinities for multi-socket systems is untested.
		* We only support asynchronous thread cancellations (i.e `PTHREAD_CANCEL_ASYNCHRONOUS`).
		* The process scope (i.e `PTHREAD_SCOPE_PROCESS`) is not supported.
		* The condition variables can currently signal upto a maximum of 64 threads simultaneously. (Limit to be removed soon.)
		* Each thread currently has maximum of 64 TLS slots. (Limit to be removed soon.)
		* Except mutexes other locking mechanisms cannot be shared across processes.
 * stdio.h
	* Functions
		* fopen, fdopen, freopen, fclose, fcloseall
		* popen, pclose
		* fileno, fileno_unlocked
		* fread, fgets, fgetc, getc, getchar
		* fread_unlocked, fgets_unlocked, fgetc_unlocked, getc_unlocked, getchar_unlocked
		* fwrite, fputs, fputc, putc, putchar
		* fwrite_unlocked, fputs_unlocked, fputc_unlocked, putc_unlocked, putchar_unlocked
		* getline, getdelim
		* fseek, fseeko, ftell, ftello, fsetpos, fgetpos, rewind, ungetc
		* fflush, fflush_unlocked, setbuf, setvbuf
		* feof, ferror, clearerr
		* feof_unlocked, ferror_unlocked, clearerr_unlocked
		* flockfile, funlockfile, ftrylockfile
		* vasprintf, vsnprintf, vasnprintf, vsprintf, asprintf, asnprintf, vdprintf, vfprintf, vprintf, snprintf, sprintf, dprintf, fprintf, printf
		* vsscanf, vfscanf, vscanf, sscanf, fscanf, scanf
		* remove, rename, renameat, renameat2
		* tempnam, tmpnam, tmpnam_r
	* Notes
		* `freopen` of the same file is implemented.
		* Line buffered and fully buffered have the same meaning here.
 * stdio_ext.h
	* Functions
		* __fbufsize, __fbufmode, __ffbf, __flbf, __fnbf
		* __freading, __fwriting, __freadable, __fwritable, __freadahead, __fpending
		* __freadptr, __fpurge, __freadptrinc
		* __fseterr, __fsetlocking
 * unistd.h
	* Functions
		* Implemented
			* alarm
			* access, eaccess, euidaccess, faccessat
			* close
			* chdir, fchdir
			* chown, lchown, fchown, fchownat
			* dup, dup2, dup3
			* fsync, fdatasync
			* getcwd
			* getuid, geteuid, setuid, seteuid
			* getgid, getegid, setgid, setegid
			* getpid, getppid, gettid
			* getdomainname, gethostname, getpagesize, getdtablesize
			* nice
			* pipe, pipe2
			* isatty
			* kill
			* link, linkat
			* read, write, pread, pwrite, lseek
			* unlink, unlinkat, rmdir, rmdirat
			* readlink, readlinkat
			* symlink, symlinkat, symlinkat2
			* sleep, msleep, usleep
			* truncate, ftruncate
			* ttyname, ttyname_r
			* sysconf, constr, pathconf, fpathconf
		* Unimplemented
			* fork, vfork
			* pause
	* Notes
		* `access`, `eaccess` perform the same operation.
		* Setting uid, gid is unsupported.
		* `symlinkat2` is an extension where the permissions of the symbolic links can be specified.
		* symlinks to special files like `/dev/null` don't work.
 * sys/acl.h
	* Functions
		* acl_init, acl_dup, acl_free, acl_valid
		* acl_copy_entry, acl_create_entry, acl_delete_entry, acl_get_entry
		* acl_add_perm, acl_clear_perms, acl_delete_perm, acl_get_perm, acl_get_permset, acl_set_permset
		* acl_add_flag, acl_clear_flags, acl_delete_flag, acl_get_flag, acl_get_flagset, acl_set_flagset
		* acl_get_qualifier, acl_set_qualifier
		* acl_get_tag_type, acl_set_tag_type
		* acl_get_fd, acl_get_file, acl_get_link, acl_set_fd, acl_set_file, acl_set_link
		* acl_calc_mask, acl_delete_def_file
	* Notes
		* Implementation of ACLs do not conform to the POSIX specification. The ACLs implemented closely model the NT kernels security descriptors.
		* ACLs have an additional flags parameter.
		* `acl_calc_mask`, `acl_delete_def_file` are no-ops.
 * sys/file.h
	* Functions
		* flock
 * sys/ioctl.h
	* Functions
		* ioctl
	* Notes
		* No ioctl codes are implemented as of now.
 * sys/mman.h
	* Functions
		* Implemented
			* mmap, munmap, mlock, munlock, mprotect, msync
		* Unsupported
			* mlockall, madvise
 * sys/mount.h
	* Functions
		* getmntinfo
 * sys/random.h
	* Functions
		* getrandom, getentropy
	* Notes
		* `getrandom` uses the `rdrand` instruction to generate random bytes.
		* `getentropy` uses the `rdseed` instruction to generate random bytes.
		* This header file needs to ported to work on ARM64 platforms.
 * sys/resource.h
	* Functions
		* getrlimit, setrlimit
		* getpriority, setpriority
		* getrusage
	* Notes
		* `getrusage` does not support getting the resource usage of children and grandchildren.
		* `setrlimit` is a no-op.
 * sys/select.h
	* Functions
		* select, pselect
		* FD_SET functions
 * sys/stat.h
	* Functions
		* Implemented
			* chmod, lchmod, fchmod, fchmodat
			* chflags, lchflags, fchflags, fchflagsat
			* stat, lstat, fstat, fstatat, statx
			* mkdir, mkdirat
			* utimens, lutimens, futimens, utimensat, fdutimens
			* umask
		* Unimplemented
			* mkfifo, mkfifoat
			* mknod, mknodat
	* Notes
		* `struct stat` has more members `st_attributes` to denote attributes, `st_birthtim` to denote creation time.
		* The `chmod` family of functions uses ACLs.
		* The `chflags` family of functions change the attributes of a file.
		* `umask` is a no-op.
 * sys/statfs.h
	* Functions
		* statfs, fstatfs
 * sys/statvfs.h
	* Functions
		* statvfs, fstatvfs
 * sys/time.h
	* Functions
		* gettimeofday
		* getitimer, setitimer
 * sys/times.h
	* Functions
		* times
 * sys/utsname.h
	* Functions
		* uname
 * sys/wait.h
 * sys/xattr.h
	* Functions
		* setxattr, lsetxattr, fsetxattr
		* getxattr, lgetxattr, fgetxattr
		* listxattr, llistxattr, flistxattr
		* removexattr, lremovexattr, fremovexattr
