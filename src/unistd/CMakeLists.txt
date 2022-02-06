#[[
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

wlibc_module(
MODULE unistd

SOURCES
access.c
chdir.c
chown.c
close.c
domainname.c
dup.c
exec.c
fsync.c
getcwd.c
getgid.c
getpid.c
getuid.c
getpagesize.c
hostname.c
isatty.c
kill.c
link.c
lseek.c
pipe.c
pread.c
pwrite.c
read.c
readlink.c
remove.c
sleep.c
symlink.c
truncate.c
ttyname.c
write.c

HEADERS
unistd.h
)
