#[[
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

wlibc_add_tests(
fdopen
fflush
fileio
fileno
fopen
freopen
getdelim
internal
pipe
rename
stream
temp
ungetc)

add_executable(pipe-helper pipe-helper.c)
target_link_libraries(pipe-helper wlibc)
set_target_properties(pipe-helper PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(pipe-helper PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR})
