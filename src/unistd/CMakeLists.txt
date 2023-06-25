#[[
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

wlibc_module(
MODULE unistd

SOURCES
access.c
alarm.c
chdir.c
chown.c
close.c
conf.c
domainname.c
dup.c
exec.c
fsync.c
getcwd.c
getdtablesize.c
getpagesize.c
gid.c
hostname.c
isatty.c
kill.c
link.c
lseek.c
nice.c
pid.c
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
uid.c
write.c

HEADERS
unistd.h
process.h
)
