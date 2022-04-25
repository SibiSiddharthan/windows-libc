/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <tests/test.h>
#include <pthread.h>

typedef struct _key_args
{
	key_t key;
	void *value;
} key_args;

int test_variable = 0;

void destructor(void *value)
{
	value = NULL;
	++test_variable;
}

void *simple(void *arg)
{
	key_args *args = (key_args *)arg;
	pthread_setspecific(args->key, args->value);
	return NULL;
}

void *simple2(void *arg)
{
	key_args *args = (key_args *)arg;
	pthread_setspecific(args->key, args->value);
	pthread_exit(NULL);
}

int test_key()
{
	int status;
	pthread_t thread;
	key_t key;
	key_args args;

	status = pthread_key_create(&key, NULL);
	ASSERT_EQ(status, 0);

	// This is the first key created.
	ASSERT_EQ(key, 0);

	status = pthread_setspecific(key, (void *)(intptr_t)1);
	ASSERT_EQ(status, 0);

	args.key = key;
	args.value = (void *)(intptr_t)5;

	status = pthread_create(&thread, NULL, simple, &args);
	ASSERT_EQ(status, 0);

	status = pthread_join(thread, NULL);
	ASSERT_EQ(status, 0);

	// This is thread local, this value should not change.
	ASSERT_EQ(pthread_getspecific(key), 1);

	status = pthread_key_delete(key);
	ASSERT_EQ(status, 0);

	// Repeated delete should result in an error.
	status = pthread_key_delete(key);
	ASSERT_EQ(status, -1);

	return 0;
}

int test_key_destructor()
{
	int status;
	pthread_t thread;
	key_t key;
	key_args args;

	status = pthread_key_create(&key, destructor);
	ASSERT_EQ(status, 0);

	args.key = key;
	args.value = NULL;

	status = pthread_create(&thread, NULL, simple, &args);
	ASSERT_EQ(status, 0);

	status = pthread_join(thread, NULL);
	ASSERT_EQ(status, 0);

	// The destructor won't be called here as the value in slot is zero.
	ASSERT_EQ(test_variable, 0);

	args.key = key;
	args.value = (void *)(intptr_t)1;

	status = pthread_create(&thread, NULL, simple, &args);
	ASSERT_EQ(status, 0);

	status = pthread_join(thread, NULL);
	ASSERT_EQ(status, 0);

	// Now the destructor will be called.
	ASSERT_EQ(test_variable, 1);

	status = pthread_key_delete(key);
	ASSERT_EQ(status, 0);

	return 0;
}

int test_key_destructor2()
{
	int status;
	pthread_t thread;
	key_t key;
	key_args args;

	status = pthread_key_create(&key, destructor);
	ASSERT_EQ(status, 0);

	args.key = key;
	args.value = NULL;

	status = pthread_create(&thread, NULL, simple2, &args);
	ASSERT_EQ(status, 0);

	status = pthread_join(thread, NULL);
	ASSERT_EQ(status, 0);

	// The destructor won't be called here as the value in slot is zero.
	ASSERT_EQ(test_variable, 0);

	args.key = key;
	args.value = (void *)(intptr_t)1;

	status = pthread_create(&thread, NULL, simple2, &args);
	ASSERT_EQ(status, 0);

	status = pthread_join(thread, NULL);
	ASSERT_EQ(status, 0);

	// Now the destructor will be called.
	ASSERT_EQ(test_variable, 1);

	status = pthread_key_delete(key);
	ASSERT_EQ(status, 0);

	return 0;
}

int test_too_many_keys()
{
	int status;
	key_t key;

	// The current implementation only supports 64 slots.
	for (int i = 0; i < 64; ++i)
	{
		status = pthread_key_create(&key, destructor);
		ASSERT_EQ(status, 0);
	}

	// Trying to create a new slot should fail.
	status = pthread_key_create(&key, destructor);
	ASSERT_EQ(status, -1);

	// Don't delete the keys here.
	return 0;
}

int main()
{
	INITIAILIZE_TESTS();

	TEST(test_key());
	TEST(test_key_destructor());
	test_variable = 0;
	// Same as above, but ensure destructors are called with pthread_exit as well.
	TEST(test_key_destructor2());
	TEST(test_too_many_keys());

	VERIFY_RESULT_AND_EXIT();
}
