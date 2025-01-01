#[[
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

wlibc_module(
MODULE poll

SOURCES
fdset.c
poll.c
select.c

HEADERS
poll.h
sys/select.h
)
