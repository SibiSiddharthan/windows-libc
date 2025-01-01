/*
   Copyright (c) 2020-2025 Sibi Siddharthan

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
typedef int pid_t;
typedef int id_t;
typedef long long int ssize_t;
typedef unsigned int dev_t;
typedef unsigned long nlink_t;
typedef unsigned short blksize_t;
typedef unsigned int blkcnt_t;
typedef unsigned long long fsblkcnt_t;
typedef unsigned long long fsfilcnt_t;
typedef long suseconds_t;
typedef unsigned long long useconds_t;

#endif