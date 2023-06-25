#[[
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

wlibc_add_tests(
access
alarm
close
chdir
chown
dup
getcwd
io
isatty
kill
link
nice
pio
pipes
symlinks
remove
truncate
ttyname)

add_executable(kill-helper kill-helper.c)
