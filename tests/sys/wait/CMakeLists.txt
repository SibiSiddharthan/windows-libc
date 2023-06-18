#[[
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

wlibc_add_tests(waitpid)

add_executable(child-helper child-helper.c)
target_link_libraries(child-helper wlibc)
