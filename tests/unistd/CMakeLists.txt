#[[
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

wlibc_add_tests(
access
close
chdir
dup
getcwd
io
isatty
kill
link
pio
symlinks
remove
truncate
ttyname)

add_executable(kill-helper kill-helper.c)
