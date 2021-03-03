#[[
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

set(signal_SOURCES
raise.c
signal.c
signal_internal.c
sigprocmask.c)

add_library(signal OBJECT ${signal_SOURCES})

install(FILES ${CMAKE_SOURCE_DIR}/include/signal-ext.h DESTINATION include)
