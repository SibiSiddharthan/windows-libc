#[[
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

set(stdio_SOURCES
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
parse_mode.c
pclose.c
perror.c
popen.c
printf.c
rename.c
scanf.c
setvbuf.c
ungetc.c
#pipe-hooks.c
#stdio-hooks.c
)

add_library(stdio OBJECT ${stdio_SOURCES})
install(FILES ${CMAKE_SOURCE_DIR}/include/stdio_ext.h DESTINATION include)
install(FILES ${CMAKE_SOURCE_DIR}/include/stdio.h DESTINATION include)
#install(FILES ${CMAKE_SOURCE_DIR}/include/stdio-hooks.h DESTINATION include)
