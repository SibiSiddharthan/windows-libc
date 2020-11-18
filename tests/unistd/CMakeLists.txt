#[[
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

set(tests
access
close
chdir
dup
getcwd
kill
link
linkat
sym_read_link
sym_read_linkat
read_write
rmdir
unlink
unlinkat)

if(NOT DEFINED CI_BUILD)
	list(APPEND tests isatty)
endif()

foreach(t ${tests})
	add_executable(test-${t} test-${t}.c)
	target_link_libraries(test-${t} wlibc)
	add_test(NAME test-${t} COMMAND test-${t} WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endforeach()

# temporary hack
set_tests_properties(test-kill PROPERTIES SKIP_RETURN_CODE 143)
