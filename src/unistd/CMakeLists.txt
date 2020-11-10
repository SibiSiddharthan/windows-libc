#[[
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT
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
isatty.c
kill.c
link.c
linkat.c
lseek.c
readlink.c
readlinkat.c
read.c
rmdir.c
symlink.c
symlinkat.c
unlink.c
unlinkat.c
write.c)

add_library(unistd OBJECT ${unistd_SOURCES})

install(FILES ${CMAKE_SOURCE_DIR}/include/unistd.h DESTINATION include)
