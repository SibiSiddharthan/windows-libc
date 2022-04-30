#[[
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

wlibc_add_tests(spawn)

add_executable(basic basic.c)
target_link_libraries(basic wlibc)

add_executable(env env.c)
target_link_libraries(env wlibc)

add_executable(arg arg.c)
target_link_libraries(arg wlibc)

add_executable(inherit-wlibc inherit.c)
target_link_libraries(inherit-wlibc wlibc)

add_executable(inherit-msvcrt inherit.c)
target_compile_definitions(inherit-msvcrt PRIVATE TESTING_MSVCRT)

set_target_properties(basic env arg inherit-wlibc inherit-msvcrt PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(basic env arg inherit-wlibc inherit-msvcrt PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR})

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/auxilary)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shebang.1 DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_executable(path path.c)
set_target_properties(path PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR}/auxilary)
set_target_properties(path PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR}/auxilary)

set_tests_properties(test-spawn PROPERTIES ENVIRONMENT "WLIBC_AUXILARY_LOCATION=${CMAKE_CURRENT_BINARY_DIR}/auxilary")
