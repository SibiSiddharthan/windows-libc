#[[
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

wlibc_module(
MODULE stdio

SOURCES
clearerr.c
fclose.c
fdopen.c
feof.c
ferror.c
fflush.c
fgetc.c
fgets.c
fileno.c
fileops.c
flock.c
fopen.c
fputc.c
fputs.c
fread.c
freopen.c
fseek.c
ftell.c
fwrite.c
getdelim.c
internal.c
mode.c
pclose.c
perror.c
popen.c
printf.c
rename.c
scanf.c
setvbuf.c
temp.c
ungetc.c
#pipe-hooks.c
#stdio-hooks.c

HEADERS
stdio.h
stdio_ext.h
#stdio-hooks.h
)
