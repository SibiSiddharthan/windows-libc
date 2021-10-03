#[[
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

set(sys_stat_SOURCES
chmod.c
fchmodat.c
fstatat.c
mkdir.c
#mkdirat.c
stat.c
utimens.c)

add_library(sys_stat OBJECT ${sys_stat_SOURCES})

install(FILES ${CMAKE_SOURCE_DIR}/include/sys/stat.h DESTINATION include/sys)
