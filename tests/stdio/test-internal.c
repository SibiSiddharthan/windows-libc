/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <stdio.h>
#include <internal/stdio.h>
#include <unistd.h>
#include <test-macros.h>

// fclose is also tested here
int test_list()
{
	int result;
	FILE *f1, *f2, *list;
	const char *filename1 = "t-list1";
	const char *filename2 = "t-list2";

	// Check std streams
	list = _wlibc_stdio_head;
	// stderr
	ASSERT_EQ(list->fd, 2);
	ASSERT_NULL(list->next);

	// stdout
	list = list->prev;
	ASSERT_EQ(list->fd, 1);
	ASSERT_EQ(list->next->fd, 2);

	// stdin
	list = list->prev;
	ASSERT_EQ(list->fd, 0);
	ASSERT_NULL(list->prev);
	ASSERT_EQ(list->next->fd, 1);

	f1 = fopen(filename1, "wD"); // delete automatically when stream is closed
	ASSERT_NOTNULL(f1);

	// reset
	list = _wlibc_stdio_head;
	ASSERT_EQ(list->fd, 3);
	ASSERT_NULL(list->next);
	ASSERT_EQ(list->prev->fd, 2);

	ASSERT_SUCCESS(fclose(stdin)); // close first stream

	list = list->prev->prev;
	ASSERT_EQ(list->fd, 1);
	ASSERT_NULL(list->prev);

	f2 = fopen(filename2, "wD"); // delete automatically when stream is closed
	ASSERT_NOTNULL(f2);

	// reset
	list = _wlibc_stdio_head;
	ASSERT_EQ(list->fd, 0); // stdin's fd should be used
	ASSERT_NULL(list->next);
	ASSERT_EQ(list->prev->fd, 3);

	ASSERT_SUCCESS(fclose(f2)); // close last stream

	// reset
	list = _wlibc_stdio_head;
	ASSERT_EQ(list->fd, 3);
	ASSERT_NULL(list->next);
	ASSERT_EQ(list->prev->fd, 2);

	ASSERT_SUCCESS(fclose(stderr)); // close something in the middle

	// reset
	list = _wlibc_stdio_head;
	ASSERT_EQ(list->fd, 3);
	ASSERT_NULL(list->next);
	ASSERT_EQ(list->prev->fd, 1); // should be stdout

	list = list->prev;
	ASSERT_NULL(list->prev);

	result = fcloseall();
	ASSERT_EQ(result, 0);

	list = _wlibc_stdio_head;
	ASSERT_NULL(list);

	// Check whether 'D' works
	ASSERT_FAIL(unlink(filename1));
	ASSERT_FAIL(unlink(filename2));

	return 0;
}

int main()
{
	// no macros for this one as we are closing all streams
	return test_list();
}
