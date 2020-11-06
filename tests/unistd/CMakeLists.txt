#[[
   Copyright (c) 2020 Sibi Siddharthan

   Distributed under MIT license.
   See file LICENSE for detail or copy at https://opensource.org/licenses/MIT
]]

set(tests
access
close
chdir
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

foreach(t ${tests})
	add_executable(test-${t} test-${t}.c)
	target_link_libraries(test-${t} wlibc)
	add_test(NAME test-${t} COMMAND test-${t} WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endforeach()

# temporary hack
set_tests_properties(test-kill PROPERTIES SKIP_RETURN_CODE 143)