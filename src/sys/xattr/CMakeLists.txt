#[[
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

wlibc_module(
MODULE sys.xattr

SOURCES
getxattr.c
listxattr.c
setxattr.c

HEADERS
sys/xattr.h
)
