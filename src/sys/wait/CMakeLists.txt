#[[
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

wlibc_module(
MODULE sys.wait

SOURCES
waitpid.c

HEADERS
sys/wait.h
)
