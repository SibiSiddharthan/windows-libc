/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_SECURITY_INTERNAL_H
#define WLIBC_SECURITY_INTERNAL_H

#include <internal/nt.h>
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

// Converting Windows permissions to Unix permissions.
#define WLIBC_ACCEPTABLE_READ_PERMISSIONS    (FILE_READ_DATA | FILE_READ_EA)
#define WLIBC_ACCEPTABLE_WRITE_PERMISSIONS   (FILE_WRITE_DATA | FILE_APPEND_DATA | FILE_WRITE_EA)
#define WLIBC_ACCEPTABLE_EXECUTE_PERMISSIONS (FILE_EXECUTE)

// Converting Unix permissions to Windows permissions.
#define WLIBC_BASIC_PERMISSIONS   (FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES | READ_CONTROL | SYNCHRONIZE | DELETE)
#define WLIBC_READ_PERMISSIONS    (FILE_READ_DATA | FILE_READ_EA | WLIBC_BASIC_PERMISSIONS)
#define WLIBC_WRITE_PERMISSIONS   (FILE_WRITE_DATA | FILE_APPEND_DATA | FILE_WRITE_EA | FILE_DELETE_CHILD | WLIBC_BASIC_PERMISSIONS)
#define WLIBC_EXECUTE_PERMISSIONS (FILE_EXECUTE | WLIBC_BASIC_PERMISSIONS)
// Only give these to system, admins and user
#define WLIBC_EXTRA_PERMISSIONS (WRITE_DAC | WRITE_OWNER) // skip ACCESS_SYSTEM_SECURITY
#define WLIBC_ALL_PERMISSIONS   (WLIBC_READ_PERMISSIONS | WLIBC_WRITE_PERMISSIONS | WLIBC_EXECUTE_PERMISSIONS | WLIBC_EXTRA_PERMISSIONS)


#endif
