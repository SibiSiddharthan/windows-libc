/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_SYS_TYPES_H
#define WLIBC_SYS_TYPES_H

typedef int mode_t;
typedef unsigned long long ino_t; // 64-bit inodes only
typedef long long int off_t;
typedef int uid_t;
typedef int gid_t;
typedef long long int ssize_t;
typedef int pid_t;
typedef unsigned int dev_t;
typedef unsigned short nlink_t;
typedef unsigned short blksize_t;
typedef unsigned int blkcnt_t;
typedef long long suseconds_t;

#endif