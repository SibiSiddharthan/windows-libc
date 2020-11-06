#[[
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT
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