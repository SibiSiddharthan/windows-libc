#[[
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

set(tests
fdopen
fflush
fileio
fileno
fopen
freopen
internal
pipe
rename
ungetc)

foreach(t ${tests})
	add_executable(test-${t} test-${t}.c)
	target_link_libraries(test-${t} wlibc)
	add_test(NAME test-${t} COMMAND test-${t} WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endforeach()

add_executable(pipe-helper pipe-helper.c)
target_link_libraries(pipe-helper wlibc)
set_target_properties(pipe-helper PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(pipe-helper PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR})
