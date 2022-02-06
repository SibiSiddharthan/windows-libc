/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <test-macros.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

int test_chflags()
{
	int fd;
	int status;
	struct stat statbuf;
	const char *filename = "t-chflags";

	fd = creat(filename, 0700);
	ASSERT_SUCCESS(close(fd));

	status = stat(filename, &statbuf);
	ASSERT_EQ(status, 0);
	// By default any file created with no attributes has FILE_ATTRIBUTE_ARCHIVE bit set.
	ASSERT_EQ((statbuf.st_attributes & S_IA_MASK), S_IA_ARCHIVE);

	status = chflags(filename, S_IA_READONLY);
	ASSERT_EQ(status, 0);

	status = stat(filename, &statbuf);
	ASSERT_EQ(status, 0);
	ASSERT_EQ((statbuf.st_attributes & S_IA_MASK), (S_IA_READONLY));

	status = chflags(filename, S_IA_READONLY | S_IA_HIDDEN);
	ASSERT_EQ(status, 0);

	status = stat(filename, &statbuf);
	ASSERT_EQ(status, 0);
	ASSERT_EQ((statbuf.st_attributes & S_IA_MASK), (S_IA_READONLY | S_IA_HIDDEN));

	status = chflags(filename, S_IA_SYSTEM);
	ASSERT_EQ(status, 0);

	status = stat(filename, &statbuf);
	ASSERT_EQ(status, 0);
	ASSERT_EQ((statbuf.st_attributes & S_IA_MASK), (S_IA_SYSTEM));

	ASSERT_SUCCESS(unlink(filename));
	return 0;
}

int test_lchflags()
{
	int fd;
	int status;
	struct stat statbuf;
	const char *filename = "t-lchflags";
	const char *filename_symlink = "t-lchflags.sym";

	fd = creat(filename, 0770);
	ASSERT_SUCCESS(close(fd));

	ASSERT_SUCCESS(symlink(filename, filename_symlink));

	status = stat(filename_symlink, &statbuf);
	ASSERT_EQ(status, 0);
	ASSERT_EQ((statbuf.st_attributes & S_IA_MASK), S_IA_ARCHIVE);

	status = lstat(filename_symlink, &statbuf);
	ASSERT_EQ(status, 0);
	ASSERT_EQ((statbuf.st_attributes & S_IA_MASK), (S_IA_ARCHIVE | S_IA_REPARSE));

	status = lchflags(filename_symlink, S_IA_HIDDEN);
	ASSERT_EQ(status, 0);

	status = lstat(filename_symlink, &statbuf);
	ASSERT_EQ(status, 0);
	ASSERT_EQ((statbuf.st_attributes & S_IA_MASK), (S_IA_HIDDEN | S_IA_REPARSE));

	// The original file's attributes should not change.
	status = stat(filename_symlink, &statbuf);
	ASSERT_EQ(status, 0);
	ASSERT_EQ((statbuf.st_attributes & S_IA_MASK), S_IA_ARCHIVE);

	ASSERT_SUCCESS(unlink(filename));
	ASSERT_SUCCESS(unlink(filename_symlink));
	return 0;
}

int test_fchflags()
{
	int fd;
	int status;
	struct stat statbuf;
	const char *filename = "t-fchflags";

	fd = creat(filename, 0700);

	status = stat(filename, &statbuf);
	ASSERT_EQ(status, 0);
	ASSERT_EQ((statbuf.st_attributes & S_IA_MASK), S_IA_ARCHIVE);

	status = fchflags(fd, S_IA_HIDDEN | S_IA_ARCHIVE);
	ASSERT_EQ(status, 0);

	status = stat(filename, &statbuf);
	ASSERT_EQ(status, 0);
	ASSERT_EQ((statbuf.st_attributes & S_IA_MASK), (S_IA_HIDDEN | S_IA_ARCHIVE));

	ASSERT_SUCCESS(close(fd));
	ASSERT_SUCCESS(unlink(filename));
	return 0;
}

void cleanup()
{
	remove("t-chflags");
	remove("t-lchflags");
	remove("t-lchflags.sym");
	remove("t-fchflags");
}

int main()
{
	INITIAILIZE_TESTS();
	CLEANUP(cleanup);

	TEST(test_chflags());
	TEST(test_lchflags());
	TEST(test_fchflags());

	VERIFY_RESULT_AND_EXIT();
}
