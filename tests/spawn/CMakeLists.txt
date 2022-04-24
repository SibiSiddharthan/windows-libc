#[[
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

wlibc_add_tests(
spawn)

add_executable(basic basic.c)
target_link_libraries(basic wlibc)

add_executable(inherit-wlibc inherit.c)
target_link_libraries(inherit-wlibc wlibc)

add_executable(inherit-msvcrt inherit.c)
target_compile_definitions(inherit-msvcrt PRIVATE TESTING_MSVCRT)

set_target_properties(basic inherit-wlibc inherit-msvcrt PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(basic inherit-wlibc inherit-msvcrt PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR})
