#[[
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

include_guard(GLOBAL)

# Function for adding tests
function(wlibc_add_tests ...)
	foreach(test ${ARGV})
		add_executable(test-${test} test-${test}.c)
		target_link_libraries(test-${test} wlibc)
		add_test(NAME test-${test} COMMAND test-${test} WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
		set_tests_properties(test-${test} PROPERTIES TIMEOUT 30)

		# In a shared build the output directory of the dll needs to be exported to PATH.
		if(BUILD_SHARED_LIBS)
			string(REPLACE ";" "\;" PATH "$ENV{PATH}")
			set_tests_properties(test-${test} PROPERTIES ENVIRONMENT "PATH=${CMAKE_BINARY_DIR}\;${PATH}")
		endif()

	endforeach()
endfunction()
