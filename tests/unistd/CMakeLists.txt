#[[
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

set(tests
access
close
chdir
dup
getcwd
isatty
kill
link
#linkat
sym_read_link
#sym_read_linkat
read_write
#rmdir
truncate
ttyname
unlink) # change this to remove
#unlinkat

foreach(t ${tests})
	add_executable(test-${t} test-${t}.c)
	target_link_libraries(test-${t} wlibc)
	add_test(NAME test-${t} COMMAND test-${t} WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endforeach()

add_executable(kill-helper kill-helper.c)
