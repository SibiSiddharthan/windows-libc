/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <internal/nt.h>
#include <internal/security.h>
#include <sys/types.h>

// Contains all possible security descriptors 0000 - 0777 * 2(directories as well)
static PISECURITY_DESCRIPTOR_RELATIVE all_security_descriptors[1024] = {NULL};

static char ntsystem_sid_buffer[SECURITY_SID_SIZE(1)];
static char adminstrators_sid_buffer[SECURITY_SID_SIZE(2)];
static char users_sid_buffer[SECURITY_SID_SIZE(2)];
static char everyone_sid_buffer[SECURITY_SID_SIZE(1)];
static char current_user_sid_buffer[SECURITY_SID_SIZE(SID_MAX_SUB_AUTHORITIES)];
static char current_group_sid_buffer[SECURITY_SID_SIZE(SID_MAX_SUB_AUTHORITIES)];
static char current_computer_sid_buffer[SECURITY_SID_SIZE(SID_MAX_SUB_AUTHORITIES)];

PISID ntsystem_sid = NULL;         // Root (NT AUTHORITY\SYSTEM)
PISID adminstrators_sid = NULL;    // psuedo root (BUILTIN\Administrators)
PISID users_sid = NULL;            // Users (BUILTIN\Users)
PISID everyone_sid = NULL;         // Everyone
PISID current_user_sid = NULL;     // Current User
PISID current_group_sid = NULL;    // Current Group
PISID current_computer_sid = NULL; // Current Computer

uid_t current_uid;
gid_t current_gid;

void initialize_sids(void)
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

	// Current User/Group/Computer
	char buffer[128];
	ULONG length_needed;
	TOKEN_ELEVATION elevation;

	// These calls will not fail. No point in checking status
	// uid
	NtQueryInformationToken(NtCurrentProcessToken(), TokenUser, buffer, 128, &length_needed);
	RtlCopySid(SECURITY_SID_SIZE(SID_MAX_SUB_AUTHORITIES), (PSID)current_user_sid_buffer, ((PTOKEN_USER)buffer)->User.Sid);
	current_user_sid = (PISID)current_user_sid_buffer;

	// computer
	// USER_SID -> COMPUTER_SID-USER_ID
	RtlCopySid(SECURITY_SID_SIZE(SID_MAX_SUB_AUTHORITIES), (PSID)current_computer_sid_buffer, ((PTOKEN_USER)buffer)->User.Sid);
	current_computer_sid = (PISID)current_computer_sid_buffer;
	current_computer_sid->SubAuthorityCount--;

	// gid
	NtQueryInformationToken(NtCurrentProcessToken(), TokenPrimaryGroup, buffer, 128, &length_needed);
	RtlCopySid(SECURITY_SID_SIZE(SID_MAX_SUB_AUTHORITIES), (PSID)current_group_sid_buffer, ((PTOKEN_PRIMARY_GROUP)buffer)->PrimaryGroup);
	current_group_sid = (PISID)current_group_sid_buffer;

	// elevation
	NtQueryInformationToken(NtCurrentProcessToken(), TokenElevation, &elevation, sizeof(TOKEN_ELEVATION), &length_needed);

	if (elevation.TokenIsElevated)
	{
		current_uid = 0; // ROOT_UID
	}
	else
	{
		// Use the last SubAuthority.
		current_uid = current_user_sid->SubAuthority[current_user_sid->SubAuthorityCount - 1];
	}

	current_gid = current_group_sid->SubAuthority[current_group_sid->SubAuthorityCount - 1];
}

void cleanup_security_decsriptors(void)
{
	for (int i = 0; i < 1024; ++i)
	{
		RtlFreeHeap(NtCurrentProcessHeap(), 0, all_security_descriptors[i]);
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
	const ULONG size_of_sd_buffer = 256;
	char *sd_buffer = (char *)RtlAllocateHeap(NtCurrentProcessHeap(), HEAP_ZERO_MEMORY, size_of_sd_buffer);

	if (sd_buffer == NULL)
	{
		errno = ENOMEM;
		return NULL;
	}

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

	// Don't add user permission if we are admin or system as we have already added them.
	if (current_uid != 0)
	{
		// Always give basic permissions to the owner.
		// Give ability to change owner and dacl to the user.
		RtlAddAccessAllowedAceEx(acl, ACL_REVISION, ace_flags,
								 determine_access_mask((mode & 0700) >> 6) | WLIBC_EXTRA_PERMISSIONS | WLIBC_BASIC_PERMISSIONS,
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
