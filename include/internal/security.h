/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_SECURITY_INTERNAL_H
#define WLIBC_SECURITY_INTERNAL_H

#include <Windows.h>
#include <winnt.h>
#include <sys/types.h>

extern PISID ntsystem_sid;         // Root (NT AUTHORITY\SYSTEM)
extern PISID adminstrators_sid;    // psuedo root (BUILTIN\Administrators)
extern PISID users_sid;            // Users (BUILTIN\Users)
extern PISID everyone_sid;         // Everyone
extern PISID current_user_sid;     // Current User
extern PISID current_group_sid;    // Current Group
extern PISID current_computer_sid; // Current Computer

extern uid_t current_uid;
extern gid_t current_gid;

void initialize_sids(void);
void cleanup_security_decsriptors(void);

PISECURITY_DESCRIPTOR_RELATIVE get_security_descriptor(mode_t mode, int is_directory);

#endif
