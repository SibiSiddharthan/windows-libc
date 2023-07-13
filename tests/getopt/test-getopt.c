/*
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <tests/test.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdlib-ext.h>

#define TEST_PROGRAM "test-getopt.exe"

int test_getopt_1()
{
	int a = 0, b = 0, c = 0;
	int ch;
	int argc = 5;
	char *argv[] = {TEST_PROGRAM, "-a", "dummy", "-b", "-c", NULL};

	optind = 0;
	while ((ch = getopt(argc, argv, "abc")) != -1)
	{
		switch (ch)
		{
		case 'a':
			a = 1;
			break;
		case 'b':
			b = 1;
			break;
		case 'c':
			c = 1;
			break;
		default:
			break;
		}
	}

	ASSERT_EQ(a, 1);
	ASSERT_EQ(b, 1);
	ASSERT_EQ(c, 1);

	return 0;
}

int test_getopt_2()
{
	int a = 0, b = 0, c = 0;
	int ch;
	int argc = 2;
	char *argv[] = {TEST_PROGRAM, "-abc", NULL};

	optind = 0;
	while ((ch = getopt(argc, argv, "abc")) != -1)
	{
		switch (ch)
		{
		case 'a':
			a = 1;
			break;
		case 'b':
			b = 1;
			break;
		case 'c':
			c = 1;
			break;
		default:
			break;
		}
	}

	ASSERT_EQ(a, 1);
	ASSERT_EQ(b, 1);
	ASSERT_EQ(c, 1);

	return 0;
}

int test_getopt_3()
{
	int a = 0, b = 0, c = 0;
	int ch;
	int argc = 5;
	char *argv[] = {TEST_PROGRAM, "-a", "dummy", "-b2", "-c", NULL};

	optind = 0;
	while ((ch = getopt(argc, argv, "ab:c")) != -1)
	{
		switch (ch)
		{
		case 'a':
			a = 1;
			break;
		case 'b':
			b = atoi(optarg);
			break;
		case 'c':
			c = 1;
			break;
		default:
			break;
		}
	}

	ASSERT_EQ(a, 1);
	ASSERT_EQ(b, 2);
	ASSERT_EQ(c, 1);

	return 0;
}

int test_getopt_4()
{
	int a = 0, b = 0, c = 0;
	int ch;
	int argc = 3;
	char *argv[] = {TEST_PROGRAM, "-ab2", "-c", NULL};

	optind = 0;
	while ((ch = getopt(argc, argv, "ab:c")) != -1)
	{
		switch (ch)
		{
		case 'a':
			a = 1;
			break;
		case 'b':
			b = atoi(optarg);
			break;
		case 'c':
			c = 1;
			break;
		default:
			break;
		}
	}

	ASSERT_EQ(a, 1);
	ASSERT_EQ(b, 2);
	ASSERT_EQ(c, 1);

	return 0;
}

int test_getopt_5()
{
	int a = 0, b = 0, c = 0;
	int ch;
	int argc = 6;
	char *argv[] = {TEST_PROGRAM, "-a", "dummy", "-b", "3", "-c", NULL};

	optind = 0;
	while ((ch = getopt(argc, argv, "ab:c")) != -1)
	{
		switch (ch)
		{
		case 'a':
			a = 1;
			break;
		case 'b':
			b = atoi(optarg);
			break;
		case 'c':
			c = 1;
			break;
		default:
			break;
		}
	}

	ASSERT_EQ(a, 1);
	ASSERT_EQ(b, 3);
	ASSERT_EQ(c, 1);

	return 0;
}

int test_getopt_6()
{
	int a = 0, b = 0, c = 0;
	int ch;
	int argc = 5;
	char *argv[] = {TEST_PROGRAM, "-a", "-b", "3", "-c4", NULL};

	optind = 0;
	while ((ch = getopt(argc, argv, "ab:c:")) != -1)
	{
		switch (ch)
		{
		case 'a':
			a = 1;
			break;
		case 'b':
			b = atoi(optarg);
			break;
		case 'c':
			c = atoi(optarg);
			break;
		default:
			break;
		}
	}

	ASSERT_EQ(a, 1);
	ASSERT_EQ(b, 3);
	ASSERT_EQ(c, 4);

	return 0;
}

int test_getopt_7()
{
	int a = 0, b = 0, c = 0, e = 0;
	int ch;
	int argc = 5;
	char *argv[] = {TEST_PROGRAM, "-a", "-b", "3", "-c", NULL};

	optind = 0;
	while ((ch = getopt(argc, argv, "ab:c:")) != -1)
	{
		switch (ch)
		{
		case 'a':
			a = 1;
			break;
		case 'b':
			b = atoi(optarg);
			break;
		case 'c':
			c = atoi(optarg);
			break;
		case '?':
			e = 1;
			break;
		default:
			break;
		}
	}

	ASSERT_EQ(a, 1);
	ASSERT_EQ(b, 3);
	ASSERT_EQ(c, 0);
	ASSERT_EQ(e, 1);

	return 0;
}

int test_getopt_8()
{
	int a = 0, b = 0, c = 0;
	int ch;
	int argc = 5;
	char *argv[] = {TEST_PROGRAM, "-a", "-b", "3", "-c", NULL};

	optind = 0;
	while ((ch = getopt(argc, argv, "ab:c::")) != -1)
	{
		switch (ch)
		{
		case 'a':
			a = 1;
			break;
		case 'b':
			b = atoi(optarg);
			break;
		case 'c':
			if (optarg)
			{
				c = atoi(optarg);
			}
			else
			{
				c = 1;
			}
			break;
		default:
			break;
		}
	}

	ASSERT_EQ(a, 1);
	ASSERT_EQ(b, 3);
	ASSERT_EQ(c, 1);

	return 0;
}

int test_getopt_9()
{
	int a = 0, b = 0, c = 0, e = 0;
	int ch;
	int argc = 5;
	char *argv[] = {TEST_PROGRAM, "-a", "-b", "3", "-d", NULL};

	optind = 0;
	while ((ch = getopt(argc, argv, "ab:c")) != -1)
	{
		switch (ch)
		{
		case 'a':
			a = 1;
			break;
		case 'b':
			b = atoi(optarg);
			break;
		case 'c':
			c = atoi(optarg);
			break;
		case '?':
			e = 1;
			ASSERT_EQ(optopt, 'd');
			break;
		default:
			break;
		}
	}

	ASSERT_EQ(a, 1);
	ASSERT_EQ(b, 3);
	ASSERT_EQ(c, 0);
	ASSERT_EQ(e, 1);

	return 0;
}

int test_getopt_10()
{
	int a = 0, b = 0, c = 0, e = 0;
	int ch;
	int argc = 5;
	char *argv[] = {TEST_PROGRAM, "-a", "-b", "3", "-d", NULL};

	optind = 0;
	while ((ch = getopt(argc, argv, ":ab:c")) != -1)
	{
		switch (ch)
		{
		case 'a':
			a = 1;
			break;
		case 'b':
			b = atoi(optarg);
			break;
		case 'c':
			c = atoi(optarg);
			break;
		case ':':
			e = 1;
			ASSERT_EQ(optopt, 'd');
			break;
		default:
			break;
		}
	}

	ASSERT_EQ(a, 1);
	ASSERT_EQ(b, 3);
	ASSERT_EQ(c, 0);
	ASSERT_EQ(e, 1);

	return 0;
}

int test_getopt_11()
{
	int a = 0, b = 0;
	int ch;
	int argc = 5;
	char *argv[] = {TEST_PROGRAM, "-a", "file", "-b", "2", NULL};

	optind = 0;
	while ((ch = getopt(argc, argv, "ab:")) != -1)
	{
		switch (ch)
		{
		case 'a':
			a = 1;
			break;
		case 'b':
			b = atoi(optarg);
			break;
		default:
			break;
		}
	}

	ASSERT_EQ(a, 1);
	ASSERT_EQ(b, 2);
	ASSERT_EQ(optind, 4);
	ASSERT_STREQ(argv[optind], "file");

	return 0;
}

int test_getopt_12()
{
	int a = 0, b = 0, e = 0;
	int ch;
	int argc = 4;
	char *argv[] = {TEST_PROGRAM, "-a", "file", "-b", NULL};

	optind = 0;
	while ((ch = getopt(argc, argv, "ab:")) != -1)
	{
		switch (ch)
		{
		case 'a':
			a = 1;
			break;
		case 'b':
			b = atoi(optarg);
			break;
		case '?':
			e = 1;
			break;
		default:
			break;
		}
	}

	ASSERT_EQ(a, 1);
	ASSERT_EQ(b, 0);
	ASSERT_EQ(e, 1);

	return 0;
}

int test_getopt_13()
{
	int a = 0, b = 0, e = 0;
	int ch;
	int argc = 4;
	char *argv[] = {TEST_PROGRAM, "-a", "--", "-b", NULL};

	optind = 0;
	while ((ch = getopt(argc, argv, "ab:")) != -1)
	{
		switch (ch)
		{
		case 'a':
			a = 1;
			break;
		case 'b':
			b = atoi(optarg);
			break;
		case '?':
			e = 1;
			break;
		default:
			break;
		}
	}

	ASSERT_EQ(a, 1);
	ASSERT_EQ(b, 0);
	ASSERT_EQ(e, 0);
	ASSERT_EQ((argc - optind), 1);

	return 0;
}

int test_getopt_14()
{
	int a = 0, b = 0, n = 0;
	int ch;
	int argc = 4;
	char *argv[] = {TEST_PROGRAM, "-a", "file", "-b2", NULL};

	optind = 0;
	while ((ch = getopt(argc, argv, "-ab:")) != -1)
	{
		switch (ch)
		{
		case '\x1':
			ASSERT_STREQ(optarg, "file");
			n = 1;
			break;
		case 'a':
			a = 1;
			break;
		case 'b':
			b = atoi(optarg);
			break;
		default:
			break;
		}
	}

	ASSERT_EQ(a, 1);
	ASSERT_EQ(b, 2);
	ASSERT_EQ(n, 1);
	ASSERT_EQ((argc - optind), 0);

	return 0;
}

int test_getopt_long_1()
{
	int a = 0, b = 0, c = 0;
	int ch;
	int argc = 5;
	char *argv[] = {TEST_PROGRAM, "-a", "dummy", "--b", "-c", NULL};
	struct option longoptions[] = {{"b", no_argument, NULL, 'b'}, {0, 0, 0, 0}};

	optind = 0;
	while ((ch = getopt_long(argc, argv, "ac", longoptions, NULL)) != -1)
	{
		switch (ch)
		{
		case 'a':
			a = 1;
			break;
		case 'b':
			b = 1;
			break;
		case 'c':
			c = 1;
			break;
		default:
			break;
		}
	}

	ASSERT_EQ(a, 1);
	ASSERT_EQ(b, 1);
	ASSERT_EQ(c, 1);

	return 0;
}

int test_getopt_long_2()
{
	int a = 0, b = 0, c = 0;
	int ch;
	int argc = 5;
	char *argv[] = {TEST_PROGRAM, "-a", "dummy", "--b=2", "-c3", NULL};
	struct option longoptions[] = {{"b", required_argument, NULL, 'b'}, {0, 0, 0, 0}};
	int index;

	optind = 0;
	while ((ch = getopt_long(argc, argv, "ac:", longoptions, &index)) != -1)
	{
		switch (ch)
		{
		case 'a':
			a = 1;
			break;
		case 'b':
			b = atoi(optarg);
			ASSERT_EQ(index, 0);
			break;
		case 'c':
			c = atoi(optarg);
			break;
		default:
			break;
		}
	}

	ASSERT_EQ(a, 1);
	ASSERT_EQ(b, 2);
	ASSERT_EQ(c, 3);

	return 0;
}

int test_getopt_long_3()
{
	int a = 0, b = 0, c = 0;
	int ch;
	int argc = 5;
	char *argv[] = {TEST_PROGRAM, "--b", "3", "--c=2", "--a", NULL};
	struct option longoptions[] = {
		{"a", no_argument, NULL, 'a'}, {"b", optional_argument, NULL, 'b'}, {"c", required_argument, NULL, 'c'}, {0, 0, 0, 0}};
	int index;

	optind = 0;
	while ((ch = getopt_long(argc, argv, "", longoptions, &index)) != -1)
	{
		switch (ch)
		{
		case 'a':
			a = 1;
			ASSERT_EQ(index, 0);
			break;
		case 'b':
			if (optarg)
			{
				b = atoi(optarg);
			}
			else
			{
				b = 1;
			}
			ASSERT_EQ(index, 1);
			break;
		case 'c':
			c = atoi(optarg);
			ASSERT_EQ(index, 2);
			break;
		default:
			break;
		}
	}

	ASSERT_EQ(a, 1);
	ASSERT_EQ(b, 3);
	ASSERT_EQ(c, 2);

	return 0;
}

int test_getopt_long_4()
{
	int a = 0, b = 0, c = 0;
	int ch;
	int argc = 4;
	char *argv[] = {TEST_PROGRAM, "--b", "--c", "--a", NULL};
	struct option longoptions[] = {
		{"a", no_argument, NULL, 'a'}, {"b", optional_argument, NULL, 'b'}, {"c", required_argument, NULL, 'c'}, {0, 0, 0, 0}};
	int index;

	optind = 0;
	while ((ch = getopt_long(argc, argv, "", longoptions, &index)) != -1)
	{
		switch (ch)
		{
		case 'a':
			a = 1;
			ASSERT_EQ(index, 0);
			break;
		case 'b':
			if (optarg)
			{
				b = atoi(optarg);
			}
			else
			{
				b = 1;
			}
			ASSERT_EQ(index, 1);
			break;
		case 'c':
			c = atoi(optarg);
			ASSERT_EQ(index, 2);
			break;
		default:
			break;
		}
	}

	ASSERT_EQ(a, 1);
	ASSERT_EQ(b, 1);
	ASSERT_EQ(c, 0);

	return 0;
}

int test_getopt_stop_1()
{
	int a = 0, b = 0, c = 0;
	int ch;
	int argc = 5;
	char *argv[] = {TEST_PROGRAM, "-a", "dummy", "-b", "-c", NULL};

	optind = 0;
	while ((ch = getopt(argc, argv, "+abc")) != -1)
	{
		switch (ch)
		{
		case 'a':
			a = 1;
			break;
		case 'b':
			b = 1;
			break;
		case 'c':
			c = 1;
			break;
		default:
			break;
		}
	}

	ASSERT_EQ(a, 1);
	ASSERT_EQ(b, 0);
	ASSERT_EQ(c, 0);

	return 0;
}

int test_getopt_stop_2()
{
	int a = 0, b = 0, c = 0;
	int ch;
	int argc = 5;
	char *argv[] = {TEST_PROGRAM, "-a", "--", "-b", "-c", NULL};

	optind = 0;
	while ((ch = getopt(argc, argv, "abc")) != -1)
	{
		switch (ch)
		{
		case 'a':
			a = 1;
			break;
		case 'b':
			b = 1;
			break;
		case 'c':
			c = 1;
			break;
		default:
			break;
		}
	}

	ASSERT_EQ(a, 1);
	ASSERT_EQ(b, 0);
	ASSERT_EQ(c, 0);

	return 0;
}

int test_getopt_posix()
{
	int a = 0, b = 0, c = 0;
	int ch;
	int argc = 5;
	char *argv[] = {TEST_PROGRAM, "-a", "dummy", "-b", "-c", NULL};

	setenv("POSIXLY_CORRECT", "YES", 1);

	optind = 0;
	while ((ch = getopt(argc, argv, "abc")) != -1)
	{
		switch (ch)
		{
		case 'a':
			a = 1;
			break;
		case 'b':
			b = 1;
			break;
		case 'c':
			c = 1;
			break;
		default:
			break;
		}
	}

	ASSERT_EQ(a, 1);
	ASSERT_EQ(b, 0);
	ASSERT_EQ(c, 0);

	return 0;
}

int main()
{
	INITIAILIZE_TESTS();

	TEST(test_getopt_1());
	TEST(test_getopt_2());
	TEST(test_getopt_3());
	TEST(test_getopt_4());
	TEST(test_getopt_5());
	TEST(test_getopt_6());
	TEST(test_getopt_7());
	TEST(test_getopt_8());
	TEST(test_getopt_9());
	TEST(test_getopt_10());
	TEST(test_getopt_11());
	TEST(test_getopt_12());
	TEST(test_getopt_13());
	TEST(test_getopt_14());

	TEST(test_getopt_long_1());
	TEST(test_getopt_long_2());
	TEST(test_getopt_long_3());
	TEST(test_getopt_long_4());

	TEST(test_getopt_stop_1());
	TEST(test_getopt_stop_2());
	TEST(test_getopt_posix());

	VERIFY_RESULT_AND_EXIT();
}
