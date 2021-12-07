#[[
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

set(signal_SOURCES
internal.c
raise.c
sigaction.c
signal.c
sigprocmask.c)

add_library(signal OBJECT ${signal_SOURCES})

install(FILES ${CMAKE_SOURCE_DIR}/include/signal.h DESTINATION include)
