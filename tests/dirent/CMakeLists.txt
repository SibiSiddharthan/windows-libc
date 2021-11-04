#[[
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

set(tests
dirent
fdopendir
opendir)

foreach(t ${tests})
	add_executable(test-${t} test-${t}.c)
	target_link_libraries(test-${t} wlibc)
	add_test(NAME test-${t} COMMAND test-${t} WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
endforeach()

set_tests_properties(test-dirent PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
