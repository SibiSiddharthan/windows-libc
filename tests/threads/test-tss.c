/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <tests/test.h>
#include <threads.h>

typedef struct _tss_args
{
	tss_t key;
	void *value;
} tss_args;

int test_variable = 0;

void destructor(void *value WLIBC_UNUSED)
{
	++test_variable;
}

int simple(void *arg)
{
	tss_args *args = (tss_args *)arg;
	tss_set(args->key, args->value);
	return 0;
}

int simple2(void *arg)
{
	tss_args *args = (tss_args *)arg;
	tss_set(args->key, args->value);
	thrd_exit(0);
}

int test_key()
{
	int status;
	thrd_t thread;
	tss_t key;
	tss_args args;

	status = tss_create(&key, NULL);
	ASSERT_EQ(status, 0);

	// This is the first key created.
	ASSERT_EQ(key, 0);

	status = tss_set(key, (void *)(intptr_t)1);
	ASSERT_EQ(status, 0);

	args.key = key;
	args.value = (void *)(intptr_t)5;

	status = thrd_create(&thread, simple, &args);
	ASSERT_EQ(status, 0);

	status = thrd_join(thread, NULL);
	ASSERT_EQ(status, 0);

	// This is thread local, this value should not change.
	ASSERT_EQ(tss_get(key), 1);

	tss_delete(key);

	return 0;
}

int test_key_destructor()
{
	int status;
	thrd_t thread;
	tss_t key;
	tss_args args;

	status = tss_create(&key, destructor);
	ASSERT_EQ(status, 0);

	args.key = key;
	args.value = NULL;

	status = thrd_create(&thread, simple, &args);
	ASSERT_EQ(status, 0);

	status = thrd_join(thread, NULL);
	ASSERT_EQ(status, 0);

	// The destructor won't be called here as the value in slot is zero.
	ASSERT_EQ(test_variable, 0);

	args.key = key;
	args.value = (void *)(intptr_t)1;

	status = thrd_create(&thread, simple, &args);
	ASSERT_EQ(status, 0);

	status = thrd_join(thread, NULL);
	ASSERT_EQ(status, 0);

	// Now the destructor will be called.
	ASSERT_EQ(test_variable, 1);

	tss_delete(key);

	return 0;
}

int test_key_destructor2()
{
	int status;
	thrd_t thread;
	tss_t key;
	tss_args args;

	status = tss_create(&key, destructor);
	ASSERT_EQ(status, 0);

	args.key = key;
	args.value = NULL;

	status = thrd_create(&thread, simple2, &args);
	ASSERT_EQ(status, 0);

	status = thrd_join(thread, NULL);
	ASSERT_EQ(status, 0);

	// The destructor won't be called here as the value in slot is zero.
	ASSERT_EQ(test_variable, 0);

	args.key = key;
	args.value = (void *)(intptr_t)1;

	status = thrd_create(&thread, simple2, &args);
	ASSERT_EQ(status, 0);

	status = thrd_join(thread, NULL);
	ASSERT_EQ(status, 0);

	// Now the destructor will be called.
	ASSERT_EQ(test_variable, 1);

	tss_delete(key);

	return 0;
}

int main()
{
	INITIAILIZE_TESTS();

	TEST(test_key());
	TEST(test_key_destructor());
	test_variable = 0;
	// Same as above, but ensure destructors are called with thrd_exit as well.
	TEST(test_key_destructor2());

	VERIFY_RESULT_AND_EXIT();
}
