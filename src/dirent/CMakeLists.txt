#[[
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

set(dirent_SOURCES
alphasort.c
opendir.c
closedir.c
dirfd.c
readdir.c
telldir.c
rewinddir.c
#scandir.c
seekdir.c
fill_dir_buffer.c)

add_library(dirent OBJECT ${dirent_SOURCES})

install(FILES ${CMAKE_SOURCE_DIR}/include/dirent.h DESTINATION include)
