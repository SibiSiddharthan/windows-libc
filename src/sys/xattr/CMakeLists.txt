#[[
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

set(sys_xattr_SOURCES
getxattr.c
listxattr.c
setxattr.c)

add_library(sys_xattr OBJECT ${sys_xattr_SOURCES})
install(FILES ${CMAKE_SOURCE_DIR}/include/sys/xattr.h DESTINATION include/sys)
