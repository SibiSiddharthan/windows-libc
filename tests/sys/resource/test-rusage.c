/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <tests/test.h>
#include <sys/resource.h>

int test_rusage()
{
	int status;
	struct rusage usage;

	status = getrusage(RUSAGE_SELF, &usage);
	ASSERT_EQ(status, 0);

	printf("Kernel time               : %lld\n", usage.ru_stime.tv_sec * 1000000 + usage.ru_stime.tv_usec);
	printf("User time                 : %lld\n", usage.ru_utime.tv_sec * 1000000 + usage.ru_utime.tv_usec);
	printf("Number of read operations : %llu\n", usage.ru_inblock);
	printf("Number of write operations: %llu\n", usage.ru_oublock);
	printf("Peak working set size     : %llu\n", usage.ru_maxrss);
	printf("Shared memory size        : %llu\n", usage.ru_ixrss);
	printf("Private memory size       : %llu\n", usage.ru_idrss);
	printf("Private memory size       : %llu\n", usage.ru_isrss);
	printf("Page reclaims             : %llu\n", usage.ru_minflt);
	printf("Page faults               : %llu\n", usage.ru_majflt);
	printf("Swap file usage           : %llu\n", usage.ru_nswap);

	return 0;
}

int main()
{
	INITIAILIZE_TESTS();
	TEST(test_rusage());
	VERIFY_RESULT_AND_EXIT();
}
