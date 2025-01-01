#[[
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

wlibc_module(
MODULE dirent

SOURCES
alphasort.c
closedir.c
dirfd.c
opendir.c
readdir.c
rewinddir.c
scandir.c
seekdir.c
telldir.c

HEADERS
dirent.h
)
