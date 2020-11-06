#[[
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT
]]

set(sys_stat_SOURCES
chmod.c
chmodat.c
mkdir.c
mkdirat.c
stat.c
statat.c)

add_library(sys_stat OBJECT ${sys_stat_SOURCES})

install(FILES ${CMAKE_SOURCE_DIR}/include/sys/stat.h DESTINATION include/sys)
