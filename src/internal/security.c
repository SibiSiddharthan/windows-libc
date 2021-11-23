/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <sys/types.h>
#include <stdlib.h>

// Contains all possible security descriptors 0000 - 0777 * 2(directories as well)
static PISECURITY_DESCRIPTOR_RELATIVE all_security_descriptors[1024] = {NULL};

static char ntsystem_sid_buffer[SECURITY_SID_SIZE(1)];
static char adminstrators_sid_buffer[SECURITY_SID_SIZE(2)];
static char users_sid_buffer[SECURITY_SID_SIZE(2)];
static char everyone_sid_buffer[SECURITY_SID_SIZE(1)];
static char current_user_sid_buffer[SECURITY_SID_SIZE(SID_MAX_SUB_AUTHORITIES)];

static PISID ntsystem_sid = NULL;      // Root (NT AUTHORITY\SYSTEM)
static PISID adminstrators_sid = NULL; // psuedo root (BUILTIN\Administrators)
static PISID users_sid = NULL;         // Users (BUILTIN\Users)
static PISID everyone_sid = NULL;      // Everyone
static PISID current_user_sid = NULL;  // Current User

#define WLIBC_BASIC_PERMISSIONS   (FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES | READ_CONTROL | SYNCHRONIZE | DELETE)
#define WLIBC_READ_PERMISSIONS    (FILE_READ_DATA | FILE_READ_EA | WLIBC_BASIC_PERMISSIONS)
#define WLIBC_WRITE_PERMISSIONS   (FILE_WRITE_DATA | FILE_APPEND_DATA | FILE_WRITE_EA | FILE_DELETE_CHILD | WLIBC_BASIC_PERMISSIONS)
#define WLIBC_EXECUTE_PERMISSIONS (FILE_EXECUTE | WLIBC_BASIC_PERMISSIONS)
// Only give these to system, admins and user
#define WLIBC_EXTRA_PERMISSIONS (WRITE_DAC | WRITE_OWNER) // skip ACCESS_SYSTEM_SECURITY
#define WLIBC_ALL_PERMISSIONS   (WLIBC_READ_PERMISSIONS | WLIBC_WRITE_PERMISSIONS | WLIBC_EXECUTE_PERMISSIONS | WLIBC_EXTRA_PERMISSIONS)

void initialize_sids()
{
	SID_IDENTIFIER_AUTHORITY nt_authority = SECURITY_NT_AUTHORITY;
	SID_IDENTIFIER_AUTHORITY world_authority = SECURITY_WORLD_SID_AUTHORITY;

	// NT AUTHORITY\SYSTEM
	RtlInitializeSidEx((PSID)ntsystem_sid_buffer, &nt_authority, 1, SECURITY_LOCAL_SYSTEM_RID);
	ntsystem_sid = (PISID)ntsystem_sid_buffer;

	// BUILTIN\Administrators
	RtlInitializeSidEx((PSID)adminstrators_sid_buffer, &nt_authority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS);
	adminstrators_sid = (PISID)adminstrators_sid_buffer;

	// BUILTIN\Users
	RtlInitializeSidEx((PSID)users_sid_buffer, &nt_authority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_USERS);
	users_sid = (PISID)users_sid_buffer;

	// Everyone
	RtlInitializeSidEx((PSID)everyone_sid_buffer, &world_authority, 1, SECURITY_WORLD_RID);
	everyone_sid = (PISID)everyone_sid_buffer;

	// Current User
	char buffer[128];
	ULONG length_needed;
	// This call would not fail. No point in checking status
	NtQuerySecurityObject(NtCurrentProcess(), OWNER_SECURITY_INFORMATION, buffer, 128, &length_needed);
	// We are only querying the owner sid, the information will be at the end of the structure.
	RtlCopySid(SECURITY_SID_SIZE(SID_MAX_SUB_AUTHORITIES), (PSID)current_user_sid_buffer,
			   (PSID)(buffer + sizeof(SECURITY_DESCRIPTOR_RELATIVE)));
	current_user_sid = (PISID)current_user_sid_buffer;
}

void cleanup_security_decsriptors()
{
	for (int i = 0; i < 1024; ++i)
	{
		free(all_security_descriptors[i]);
	}
}

static PISECURITY_DESCRIPTOR_RELATIVE lookup_cache(mode_t mode, int is_directory)
{
	return all_security_descriptors[(mode & 511) | (is_directory == 0 ? 0 : 512)];
}

static void insert_cache(PISECURITY_DESCRIPTOR_RELATIVE sid, mode_t mode, int is_directory)
{
	all_security_descriptors[(mode & 511) | (is_directory == 0 ? 0 : 512)] = sid;
}

static ACCESS_MASK determine_access_mask(mode_t mode)
{
	ACCESS_MASK access = 0;

	if (mode & 0001)
	{
		access |= WLIBC_EXECUTE_PERMISSIONS;
	}
	if (mode & 0002)
	{
		access |= WLIBC_WRITE_PERMISSIONS;
	}
	if (mode & 0004)
	{
		access |= WLIBC_READ_PERMISSIONS;
	}

	return access;
}

static PISECURITY_DESCRIPTOR_RELATIVE create_security_descriptor(mode_t mode, int is_directory)
{
	/*
	  Size of SECURITY_DESCRIPTOR_RELATIVE : 20
	  Size of ACL Header                   : 8
	  Size of (NT AUTHORITY\SYSTEM)ACE     : 20 (1 subauthorities)
	  Size of (BUILTIN\Administrators) ACE : 24 (2 subauthorities)
	  Size of (BUILTIN\Users) ACE          : 24 (2 subauthorities)
	  Size of (Everyone) ACE               : 20 (1 subauthorities)
	  Size of User ACE                     : 36 (5 subauthorities usually) or 76 (15 max subauthorities)
	  Total                                : 152 bytes or 192 bytes max
	*/
	const size_t size_of_sd_buffer = 256;
	char *sd_buffer = (char *)malloc(size_of_sd_buffer);
	memset(sd_buffer, 0, size_of_sd_buffer);

	PISECURITY_DESCRIPTOR_RELATIVE security_descriptor = (PISECURITY_DESCRIPTOR_RELATIVE)sd_buffer;
	security_descriptor->Revision = SECURITY_DESCRIPTOR_REVISION;
	security_descriptor->Control = SE_OWNER_DEFAULTED | SE_GROUP_DEFAULTED | SE_DACL_PRESENT | SE_SELF_RELATIVE;
	// No need to set other fields to zero as we are memsetting anyway.
	// Put the DACL at the end.
	security_descriptor->Dacl = sizeof(SECURITY_DESCRIPTOR_RELATIVE);

	PACL acl = (PACL)(sd_buffer + security_descriptor->Dacl);
	RtlCreateAcl(acl, size_of_sd_buffer - sizeof(SECURITY_DESCRIPTOR_RELATIVE), ACL_REVISION);

	ULONG ace_flags = (is_directory == 0 ? 0 : OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE);
	// Give adminstrators and the system full permissions always.
	RtlAddAccessAllowedAceEx(acl, ACL_REVISION, ace_flags, WLIBC_ALL_PERMISSIONS, (PSID)ntsystem_sid);
	RtlAddAccessAllowedAceEx(acl, ACL_REVISION, ace_flags, WLIBC_ALL_PERMISSIONS, (PSID)adminstrators_sid);

	if (mode & 0700)
	{
		// Give ability to change owner and dacl to the user.
		RtlAddAccessAllowedAceEx(acl, ACL_REVISION, ace_flags, determine_access_mask((mode & 0700) >> 6) | WLIBC_EXTRA_PERMISSIONS,
								 (PSID)current_user_sid);
	}
	if (mode & 0070)
	{
		RtlAddAccessAllowedAceEx(acl, ACL_REVISION, ace_flags, determine_access_mask((mode & 0070) >> 3), (PSID)users_sid);
	}
	if (mode & 0007)
	{
		RtlAddAccessAllowedAceEx(acl, ACL_REVISION, ace_flags, determine_access_mask(mode & 0007), (PSID)everyone_sid);
	}

	insert_cache((PSID)security_descriptor, mode, is_directory);
	return (PSID)security_descriptor;
}

PISECURITY_DESCRIPTOR_RELATIVE get_security_descriptor(mode_t mode, int is_directory)
{
	PSID security_descriptor = lookup_cache(mode, is_directory);
	if (security_descriptor == NULL)
	{
		security_descriptor = create_security_descriptor(mode, is_directory);
	}

	return security_descriptor;
}
