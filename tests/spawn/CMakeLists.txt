#[[
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

wlibc_add_tests(spawn)

add_executable(basic basic.c)
target_link_libraries(basic wlibc)

add_executable(env env.c)
target_link_libraries(env wlibc)

add_executable(cwd cwd.c)
target_link_libraries(cwd wlibc)

add_executable(arg arg.c)
target_link_libraries(arg wlibc)

add_executable(inherit-wlibc inherit.c)
target_link_libraries(inherit-wlibc wlibc)

add_executable(inherit-msvcrt inherit.c)
target_compile_definitions(inherit-msvcrt PRIVATE TESTING_MSVCRT)

set_target_properties(basic env cwd arg inherit-wlibc inherit-msvcrt PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(basic env cwd arg inherit-wlibc inherit-msvcrt PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR})

add_executable(simple simple.c)

add_executable(arg-shebang arg.c)
target_link_libraries(arg-shebang wlibc)

add_executable(arg-shebang-spaces arg.c)
target_link_libraries(arg-shebang-spaces wlibc)
set_target_properties(arg-shebang-spaces PROPERTIES RUNTIME_OUTPUT_NAME "arg shebang")

set_target_properties(simple arg-shebang arg-shebang-spaces PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR}/auxilary)
set_target_properties(simple arg-shebang arg-shebang-spaces PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR}/auxilary)
