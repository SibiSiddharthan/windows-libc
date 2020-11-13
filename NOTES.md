## General
 * POSIX permissions are not supported yet. `chmod()` behaves in the same manner as in msvcrt, it only denies write permission to files by setting them to be read-only.
 * `fork()` and `vfork()` will never be implemented unless there is a native system call that helps it.
 * For the `POSIX_IO` module we maintain our own file descriptors apart from the ones maintained by `msvcrt`.
 * If a static library of wlibc is used, We provide `wmain()` which will used as the entry point.

### Modules
 * DLFCN
	* The second argument to `dlopen()` is ignored.
 * POSIX_IO
	* `open()` and `fopen()` provide redirection to `/dev/null` and `/dev/tty` as `NUL` and `CON` respectively.
	* We provide hooks to `fopen()`, `fclose()`, `fdopen()`, `freopen()`, `fileno()` functions for interoperabilty with our `read()` and `write()` functions.
	* `read()` and `write()` don't do `lf` to `crlf` conversions or vice versa. So, if you choose to use `read()` or `write` with `FILE*` streams open them in binary mode beforehand.
	* `fopen()` and `fdopen()` do not open directories.
	* Only one `DIR*` stream can be opened with `fdopendir()`.
	* The `d_off` member of `struct dirent` holds the value of the offset of the current file in a directory from the first entry of the directory(`.`).
	* For unresolved symlinks, the `lstat()` function sets `st_size` parameter to `0`, instead of the size of the supposed link text.
	* The `stat` structure includes `st_blocks` and `st_blksize` members.
	* The `chown()` functions don't do anything and just return `0`.
	* The `mode` argument to `mkdir()` is ignored.
	* Our `rename()` is POSIX compliant.
	* Our `unlink()` removes read-only files also.
	* Operations involving `AT_EMPTY_PATH` flag are unsupported.
