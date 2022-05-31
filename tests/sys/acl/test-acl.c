/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <tests/test.h>
#include <fcntl.h>
#include <sys/acl.h>
#include <sys/stat.h>
#include <unistd.h>

int test_acl()
{
	int status;
	int fd;
	struct stat statbuf;
	acl_t acl = NULL, acldup = NULL;
	const char *file_source = "t-acl-source";
	const char *file_destination = "t-acl-destination";

	fd = open(file_source, O_CREAT | O_RDWR | O_TRUNC, 0770);
	ASSERT_NOTEQ(fd, -1);
	ASSERT_SUCCESS(close(fd));

	ASSERT_SUCCESS(stat(file_source, &statbuf));
	ASSERT_EQ(statbuf.st_mode, (S_IFREG | 0770))

	fd = open(file_destination, O_CREAT | O_RDWR | O_TRUNC, 0700);
	ASSERT_NOTEQ(fd, -1);
	ASSERT_SUCCESS(close(fd));

	ASSERT_SUCCESS(stat(file_destination, &statbuf));
	ASSERT_EQ(statbuf.st_mode, (S_IFREG | 0700))

	acl = acl_get_file(file_source, ACL_TYPE_EXTENDED);
	ASSERT_NOTNULL(acl);

	acldup = acl_dup(acl);
	ASSERT_NOTNULL(acldup);

	status = acl_set_file(file_destination, ACL_TYPE_EXTENDED, acldup);
	ASSERT_EQ(status, 0);

	// We have copied the acl of source to destination. The mode of both of them should be same now.
	ASSERT_SUCCESS(stat(file_destination, &statbuf));
	ASSERT_EQ(statbuf.st_mode, (S_IFREG | 0770))

	ASSERT_EQ(acl_free(acl), 0);
	ASSERT_EQ(acl_free(acldup), 0);

	ASSERT_SUCCESS(unlink(file_source));
	ASSERT_SUCCESS(unlink(file_destination));

	return 0;
}

int test_entry_file()
{
	int status;
	int fd;
	struct stat statbuf;
	acl_t acl = NULL;
	acl_entry_t entry = NULL;
	acl_flagset_t flagset = NULL;
	acl_permset_t permset = NULL;
	acl_tag_t tag;
	const char *file = "t-entry.file";

	fd = open(file, O_CREAT | O_RDWR | O_TRUNC, 0750);
	ASSERT_NOTEQ(fd, -1);

	ASSERT_SUCCESS(stat(file, &statbuf));
	ASSERT_EQ(statbuf.st_mode, (S_IFREG | 0750))

	acl = acl_get_fd(fd);
	ASSERT_NOTNULL(acl);

	// Test the permissions, flags, type for the 'BUILTIN\Users'. It will be the last one.
	status = acl_get_entry(acl, ACL_LAST_ENTRY, &entry);
	ASSERT_EQ(status, 0);

	status = acl_get_flagset(entry, &flagset);
	ASSERT_EQ(status, 0);

	ASSERT_EQ(acl_get_flag(flagset, ACL_OBJECT_INHERIT), 0);
	ASSERT_EQ(acl_get_flag(flagset, ACL_CONTAINER_INHERIT), 0);

	status = acl_get_permset(entry, &permset);
	ASSERT_EQ(status, 0);

	status = acl_get_tag_type(entry, &tag);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(tag, ACL_EXTENDED_ALLOW);

	ASSERT_EQ(acl_get_perm(permset, ACL_READ), 1);
	ASSERT_EQ(acl_get_perm(permset, ACL_WRITE), 0);
	ASSERT_EQ(acl_get_perm(permset, ACL_EXECUTE), 1);

	status = acl_add_perm(permset, ACL_WRITE);
	ASSERT_EQ(status, 0);

	status = acl_set_permset(entry, permset);
	ASSERT_EQ(status, 0);

	// Querying after last entry should set entry as NULL.
	status = acl_get_entry(acl, ACL_NEXT_ENTRY, &entry);
	ASSERT_EQ(status, 0);
	ASSERT_NULL(entry);

	// Give 'BUILTIN\Users' all permissions.
	status = acl_set_fd(fd, acl);
	ASSERT_EQ(status, 0);

	ASSERT_SUCCESS(stat(file, &statbuf));
	ASSERT_EQ(statbuf.st_mode, (S_IFREG | 0770))

	ASSERT_EQ(acl_free(acl), 0);

	ASSERT_SUCCESS(close(fd));
	ASSERT_SUCCESS(unlink(file));

	return 0;
}

int test_entry_dir()
{
	int status;
	struct stat statbuf;
	acl_t acl = NULL;
	acl_entry_t entry = NULL;
	acl_flagset_t flagset = NULL;
	acl_permset_t permset = NULL;
	acl_tag_t tag;
	const char *directory = "t-entry.dir";

	ASSERT_SUCCESS(mkdir(directory, 0750));

	ASSERT_SUCCESS(stat(directory, &statbuf));
	ASSERT_EQ(statbuf.st_mode, (S_IFDIR | 0750))

	acl = acl_get_file(directory, ACL_TYPE_EXTENDED);
	ASSERT_NOTNULL(acl);

	// Test the permissions, flags, type for the 'BUILTIN\Users'. It will be the last one.
	status = acl_get_entry(acl, ACL_LAST_ENTRY, &entry);
	ASSERT_EQ(status, 0);

	status = acl_get_flagset(entry, &flagset);
	ASSERT_EQ(status, 0);

	ASSERT_EQ(acl_get_flag(flagset, ACL_OBJECT_INHERIT), 1);
	ASSERT_EQ(acl_get_flag(flagset, ACL_CONTAINER_INHERIT), 1);

	status = acl_get_permset(entry, &permset);
	ASSERT_EQ(status, 0);

	ASSERT_EQ(acl_get_perm(permset, ACL_READ), 1);
	ASSERT_EQ(acl_get_perm(permset, ACL_WRITE), 0);
	ASSERT_EQ(acl_get_perm(permset, ACL_EXECUTE), 1);

	status = acl_get_tag_type(entry, &tag);
	ASSERT_EQ(status, 0);
	ASSERT_EQ(tag, ACL_EXTENDED_ALLOW);

	status = acl_delete_entry(acl, entry);
	ASSERT_EQ(status, 0);

	// Querying after deleting last entry should return an error.
	errno = 0;
	status = acl_get_entry(acl, ACL_NEXT_ENTRY, &entry);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(ESRCH);

	status = acl_set_file(directory, ACL_TYPE_EXTENDED, acl);
	ASSERT_EQ(status, 0);

	ASSERT_SUCCESS(stat(directory, &statbuf));
	ASSERT_EQ(statbuf.st_mode, (S_IFDIR | 0700))

	ASSERT_EQ(acl_free(acl), 0);

	ASSERT_SUCCESS(rmdir(directory));

	return 0;
}

int test_copy()
{
	int status;
	int fd;
	struct stat statbuf;
	acl_t acl_source = NULL, acl_destination = NULL;
	acl_entry_t entry_source = NULL, entry_destination = NULL;
	const char *file_source = "t-acl-copy-source";
	const char *file_destination = "t-acl-copy-destination";

	fd = open(file_source, O_CREAT | O_RDWR | O_TRUNC, 0770);
	ASSERT_NOTEQ(fd, -1);
	ASSERT_SUCCESS(close(fd));

	ASSERT_SUCCESS(stat(file_source, &statbuf));
	ASSERT_EQ(statbuf.st_mode, (S_IFREG | 0770))

	fd = open(file_destination, O_CREAT | O_RDWR | O_TRUNC, 0740);
	ASSERT_NOTEQ(fd, -1);
	ASSERT_SUCCESS(close(fd));

	ASSERT_SUCCESS(stat(file_destination, &statbuf));
	ASSERT_EQ(statbuf.st_mode, (S_IFREG | 0740))

	acl_source = acl_get_file(file_source, ACL_TYPE_EXTENDED);
	ASSERT_NOTNULL(acl_source);

	acl_destination = acl_get_file(file_destination, ACL_TYPE_EXTENDED);
	ASSERT_NOTNULL(acl_destination);

	status = acl_get_entry(acl_source, ACL_LAST_ENTRY, &entry_source);
	ASSERT_EQ(status, 0);

	status = acl_get_entry(acl_destination, ACL_LAST_ENTRY, &entry_destination);
	ASSERT_EQ(status, 0);

	status = acl_copy_entry(entry_destination, entry_source);
	ASSERT_EQ(status, 0);

	status = acl_set_file(file_destination, ACL_TYPE_EXTENDED, acl_destination);
	ASSERT_EQ(status, 0);

	// We have copied the acl of source to destination. The mode of both of them should be same now.
	ASSERT_SUCCESS(stat(file_destination, &statbuf));
	ASSERT_EQ(statbuf.st_mode, (S_IFREG | 0770))

	ASSERT_EQ(acl_free(acl_source), 0);
	ASSERT_EQ(acl_free(acl_destination), 0);

	ASSERT_SUCCESS(unlink(file_source));
	ASSERT_SUCCESS(unlink(file_destination));

	return 0;
}

int test_create()
{
	int status;
	int fd;
	struct stat statbuf;
	acl_t acl_source = NULL, acl_destination = NULL;
	acl_entry_t entry_source = NULL, entry_destination = NULL;
	acl_flagset_t flagset = NULL;
	acl_permset_t permset = NULL;
	acl_qualifier_t qualifier = NULL;
	acl_tag_t tag;
	const char *file_source = "t-acl-create-source";
	const char *file_destination = "t-acl-create-destination";

	fd = open(file_source, O_CREAT | O_RDWR | O_TRUNC, 0770);
	ASSERT_NOTEQ(fd, -1);
	ASSERT_SUCCESS(close(fd));

	ASSERT_SUCCESS(stat(file_source, &statbuf));
	ASSERT_EQ(statbuf.st_mode, (S_IFREG | 0770))

	fd = open(file_destination, O_CREAT | O_RDWR | O_TRUNC, 0700);
	ASSERT_NOTEQ(fd, -1);
	ASSERT_SUCCESS(close(fd));

	ASSERT_SUCCESS(stat(file_destination, &statbuf));
	ASSERT_EQ(statbuf.st_mode, (S_IFREG | 0700))

	acl_source = acl_get_file(file_source, ACL_TYPE_EXTENDED);
	ASSERT_NOTNULL(acl_source);

	acl_destination = acl_get_file(file_destination, ACL_TYPE_EXTENDED);
	ASSERT_NOTNULL(acl_destination);

	status = acl_get_entry(acl_source, ACL_LAST_ENTRY, &entry_source);
	ASSERT_EQ(status, 0);

	// Create a new entry.
	status = acl_create_entry(&acl_destination, &entry_destination);
	ASSERT_EQ(status, 0);

	// Copy permset
	status = acl_get_permset(entry_source, &permset);
	ASSERT_EQ(status, 0);

	status = acl_set_permset(entry_destination, permset);
	ASSERT_EQ(status, 0);

	// Copy flagset
	status = acl_get_flagset(entry_source, &flagset);
	ASSERT_EQ(status, 0);

	status = acl_set_flagset(entry_destination, flagset);
	ASSERT_EQ(status, 0);

	// Copy tag
	status = acl_get_tag_type(entry_source, &tag);
	ASSERT_EQ(status, 0);

	status = acl_set_tag_type(entry_destination,tag);
	ASSERT_EQ(status, 0);

	// Copy qualifier
	qualifier = acl_get_qualifier(entry_source);
	ASSERT_NOTNULL(qualifier);

	status = acl_set_qualifier(entry_destination, qualifier);
	ASSERT_EQ(status, 0);

	status = acl_set_file(file_destination, ACL_TYPE_EXTENDED, acl_destination);
	ASSERT_EQ(status, 0);

	// We have copied the acl of source to destination. The mode of both of them should be same now.
	ASSERT_SUCCESS(stat(file_destination, &statbuf));
	ASSERT_EQ(statbuf.st_mode, (S_IFREG | 0770))

	ASSERT_EQ(acl_free(acl_source), 0);
	ASSERT_EQ(acl_free(acl_destination), 0);

	ASSERT_SUCCESS(unlink(file_source));
	ASSERT_SUCCESS(unlink(file_destination));

	return 0;
}

void cleanup()
{
	remove("t-acl-source");
	remove("t-acl-destination");
	remove("t-entry.file");
	remove("t-entry.dir");
	remove("t-acl-copy-source");
	remove("t-acl-copy-destination");
	remove("t-acl-create-source");
	remove("t-acl-create-destination");
}

int main()
{
	INITIAILIZE_TESTS();
	CLEANUP(cleanup);

	TEST(test_acl());
	TEST(test_entry_file());
	TEST(test_entry_dir());
	TEST(test_copy());
	TEST(test_create());

	VERIFY_RESULT_AND_EXIT();
}
