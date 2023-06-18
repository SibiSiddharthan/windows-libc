#[[
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

wlibc_module(
MODULE sys.acl

SOURCES
acl.c
entry.c
flags.c
object.c
perm.c
qualifier.c
tag.c

HEADERS
sys/acl.h
)
