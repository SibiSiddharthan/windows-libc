#[[
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

wlibc_module(
MODULE signal

SOURCES
internal.c
raise.c
sigaction.c
signal.c
sigprocmask.c

HEADERS
signal.h
)
