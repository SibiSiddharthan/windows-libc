/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <test-macros.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/xattr.h>
#include <unistd.h>

int test_xattr()
{
	ssize_t status;
	int fd;
	char buf[16];
	const char *filename = "t-xattr";

	fd = creat(filename, 0700);
	ASSERT_SUCCESS(close(fd));

	// setxattr tests
	status = setxattr(filename, "attr1", "val1", 4, XATTR_CREATE);
	ASSERT_EQ(status, 0);

	errno = 0;
	// Creating an attribute that already exists should fail
	status = setxattr(filename, "attr1", "val1", 4, XATTR_CREATE);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(EEXIST);

	errno = 0;
	// Replacing an attribute that does not exist should fail
	status = setxattr(filename, "attr2", "val2", 4, XATTR_REPLACE);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(ENODATA);

	// getxattr tests
	// Get the minimum size required
	status = getxattr(filename, "attr1", NULL, 0);
	ASSERT_EQ(status, 4);

	errno = 0;
	status = getxattr(filename, "attr1", buf, 2);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(ERANGE);

	memset(buf, 0, 16);
	status = getxattr(filename, "attr1", buf, 16);
	ASSERT_EQ(status, 4);
	ASSERT_MEMEQ(buf, "val1", 4);

	errno = 0;
	status = getxattr(filename, "attr2", buf, 2);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(ENODATA);

	// This should succeed always
	status = setxattr(filename, "attr2", "val2", 4, 0);
	ASSERT_EQ(status, 0);

	memset(buf, 0, 16);
	status = getxattr(filename, "attr2", buf, 16);
	ASSERT_EQ(status, 4);
	ASSERT_MEMEQ(buf, "val2", 4);

	status = setxattr(filename, "attr2", "val3", 4, 0);
	ASSERT_EQ(status, 0);

	memset(buf, 0, 16);
	status = getxattr(filename, "attr2", buf, 16);
	ASSERT_EQ(status, 4);
	ASSERT_MEMEQ(buf, "val3", 4);

	status = setxattr(filename, "attr1", "val4", 4, XATTR_REPLACE);
	ASSERT_EQ(status, 0);

	memset(buf, 0, 16);
	status = getxattr(filename, "attr1", buf, 16);
	ASSERT_EQ(status, 4);
	ASSERT_MEMEQ(buf, "val4", 4);

	// listxattr tests
	status = listxattr(filename, NULL, 0);
	ASSERT_EQ(status, 12);

	errno = 0;
	status = listxattr(filename, buf, 4);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(ERANGE)

	memset(buf, 0, 16);
	status = listxattr(filename, buf, 16);
	ASSERT_EQ(status, 12);
	// NTFS extended attribute names are all uppercase
	ASSERT_MEMEQ(buf, "ATTR2\0ATTR1\0", 12);

	// removexattr tests
	errno = 0;
	// Trying to remove a non existent attribute should fail
	status = removexattr(filename, "attr3");
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(ENODATA);

	status = removexattr(filename, "attr1");
	ASSERT_EQ(status, 0);

	errno = 0;
	status = getxattr(filename, "attr1", buf, 16);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(ENODATA);

	status = removexattr(filename, "attr2");
	ASSERT_EQ(status, 0);

	errno = 0;
	status = getxattr(filename, "attr2", buf, 16);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(ENODATA);

	ASSERT_SUCCESS(unlink(filename));
	return 0;
}

int test_xattr_dir()
{
	ssize_t status;
	char buf[16];
	const char *dirname = "t-xattr.dir";

	ASSERT_SUCCESS(mkdir(dirname, 0700));

	status = setxattr(dirname, "attr1", "val1", 4, XATTR_CREATE);
	ASSERT_EQ(status, 0);

	memset(buf, 0, 16);
	status = getxattr(dirname, "attr1", buf, 16);
	ASSERT_EQ(status, 4);
	ASSERT_MEMEQ(buf, "val1", 4);

	memset(buf, 0, 16);
	status = listxattr(dirname, buf, 16);
	ASSERT_EQ(status, 6);
	ASSERT_MEMEQ(buf, "ATTR1\0", 6);

	status = removexattr(dirname, "attr1");
	ASSERT_EQ(status, 0);

	errno = 0;
	status = getxattr(dirname, "attr1", buf, 16);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(ENODATA);

	ASSERT_SUCCESS(rmdir(dirname));
	return 0;
}

int test_lxattr()
{
	ssize_t status;
	int fd;
	char buf[16];
	const char *filename = "t-lxattr";
	const char *filename_symlink = "t-lxattr.sym";

	fd = creat(filename, 0700);
	ASSERT_SUCCESS(close(fd));

	ASSERT_SUCCESS(symlink(filename, filename_symlink));

	status = lsetxattr(filename_symlink, "attr1", "val1", 4, XATTR_CREATE);
	ASSERT_EQ(status, 0);

	memset(buf, 0, 16);
	status = lgetxattr(filename_symlink, "attr1", buf, 16);
	ASSERT_EQ(status, 4);
	ASSERT_MEMEQ(buf, "val1", 4);

	// The real file should not have any ea
	errno = 0;
	status = lgetxattr(filename, "attr1", buf, 2);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(ENODATA);

	memset(buf, 0, 16);
	status = llistxattr(filename_symlink, buf, 16);
	ASSERT_EQ(status, 6);
	ASSERT_MEMEQ(buf, "ATTR1\0", 6);

	// The real file should not have any ea, verify with llistxattr as well
	status = llistxattr(filename, buf, 16);
	ASSERT_EQ(status, 0);

	status = lremovexattr(filename_symlink, "attr1");
	ASSERT_EQ(status, 0);

	errno = 0;
	status = lgetxattr(filename_symlink, "attr1", buf, 16);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(ENODATA);

	ASSERT_SUCCESS(unlink(filename));
	ASSERT_SUCCESS(unlink(filename_symlink));
	return 0;
}

int test_fxattr()
{
	ssize_t status;
	int fd;
	char buf[16];
	const char *filename = "t-fxattr";

	fd = creat(filename, 0700);

	status = fsetxattr(fd, "attr1", "val1", 4, XATTR_CREATE);
	ASSERT_EQ(status, 0);

	memset(buf, 0, 16);
	status = fgetxattr(fd, "attr1", buf, 16);
	ASSERT_EQ(status, 4);
	ASSERT_MEMEQ(buf, "val1", 4);

	memset(buf, 0, 16);
	status = flistxattr(fd, buf, 16);
	ASSERT_EQ(status, 6);
	ASSERT_MEMEQ(buf, "ATTR1\0", 6);

	status = fremovexattr(fd, "attr1");
	ASSERT_EQ(status, 0);

	errno = 0;
	status = fgetxattr(fd, "attr1", buf, 16);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(ENODATA);

	ASSERT_SUCCESS(close(fd));
	ASSERT_SUCCESS(unlink(filename));

	return 0;
}

int test_big_attr()
{
	ssize_t status;
	int fd;
	char name[512], list[512];
	char *value = NULL, *buffer = NULL;
	const char *filename = "t-xattr-big";

	fd = creat(filename, 0700);
	ASSERT_SUCCESS(close(fd));

	// Don't worry about memory leaks here
	value = (char *)malloc(100000);
	buffer = (char *)malloc(100000);

	errno = 0;
	// size of value > 65536
	status = setxattr(filename, "attr1", value, 100000, 0);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(E2BIG);

	errno = 0;
	status = getxattr(filename, "attr1", buffer, 2);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(ENODATA);

	errno = 0;
	memset(name, 'a', 512);
	// size of name > 256
	status = setxattr(filename, name, value, 100000, 0);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(E2BIG);

	memset(name, 0, 512);
	memset(name, '*', 254); // invalid character

	errno = 0;
	status = setxattr(filename, name, "val1", 4, 0);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(EINVAL);

	memset(name, 0, 512);
	memset(name, 'a', 254);
	memset(value, 1, 32768);
	memset(buffer, 0, 32768);

	status = setxattr(filename, name, value, 32768, 0);
	ASSERT_EQ(status, 0);

	errno = 0;
	status = setxattr(filename, name, value, 32768, XATTR_CREATE);
	ASSERT_EQ(status, -1);
	ASSERT_ERRNO(EEXIST);

	status = getxattr(filename, name, buffer, 100000);
	ASSERT_EQ(status, 32768);
	ASSERT_MEMEQ(buffer, value, 32768);

	status = listxattr(filename, list, 512);
	ASSERT_EQ(status, 254 + 1);
	if (stricmp(list, name) != 0)
	{
		printf("Assertion failed at %s:%d in %s. Case insensitive string comparsion failed. Expected string\n%s\nActual string\n%s\n",
			   __FILE__, __LINE__, __FUNCTION__, name, list);
		return 1;
	}

	status = removexattr(filename, name);
	ASSERT_EQ(status, 0);

	ASSERT_SUCCESS(unlink(filename));
	free(value);
	free(buffer);

	return 0;
}

void cleanup()
{
	remove("t-xattr");
	remove("t-xattr.dir");
	remove("t-lxattr");
	remove("t-lxattr.sym");
	remove("t-fxattr");
	remove("t-xattr-big");
}

int main()
{
	INITIAILIZE_TESTS();
	CLEANUP(cleanup);

	TEST(test_xattr());
	TEST(test_xattr_dir());
	TEST(test_lxattr());
	TEST(test_fxattr());
	TEST(test_big_attr());

	VERIFY_RESULT_AND_EXIT();
}
