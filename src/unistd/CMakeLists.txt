#[[
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

set(unistd_SOURCES
access.c
close.c
chdir.c
chown.c
dup.c
faccessat.c
fchownat.c
fsync.c
getcwd.c
getgid.c
getpid.c
getuid.c
getpagesize.c
isatty.c
kill.c
link.c
linkat.c
lseek.c
pipe.c
readlink.c
readlinkat.c
read.c
rmdir.c
symlink.c
symlinkat.c
truncate.c
ttyname.c
unlink.c
unlinkat.c
write.c)

add_library(unistd OBJECT ${unistd_SOURCES})

install(FILES ${CMAKE_SOURCE_DIR}/include/unistd.h DESTINATION include)
