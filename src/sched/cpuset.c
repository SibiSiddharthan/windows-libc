/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#include <errno.h>
#include <sched.h>
#include <stdlib.h>
#include <intrin.h>

size_t wlibc_cpu_alloc_size(int num_cpus)
{
	if (num_cpus <= 0)
	{
		errno = EINVAL;
		return 0;
	}

	// Each group holds up to 64 logical processors.
	// 4 bytes for number of groups.
	// 4 bytes for number of cpus.
	// 8 * number of groups.
	return sizeof(cpu_set_t) + ((num_cpus - 1) / 64) * 8;
}

cpu_set_t *wlibc_cpu_alloc(int num_cpus)
{
	if (num_cpus <= 0)
	{
		errno = EINVAL;
		return NULL;
	}

	cpu_set_t *set = (cpu_set_t *)malloc(sizeof(cpu_set_t) + ((num_cpus - 1) / 64) * 8);
	set->num_groups = (num_cpus / 64) + ((num_cpus % 64) == 0 ? 0 : 1);
	set->num_cpus = num_cpus;

	for (int i = 0; i < set->num_groups; ++i)
	{
		set->group_mask[i] = 0;
	}

	return set;
}

void wlibc_cpu_free(cpu_set_t *set)
{
	free(set);
}

void wlibc_cpu_zero(cpu_set_t *set)
{
	if (set == NULL)
	{
		errno = EINVAL;
		return;
	}

	for (int i = 0; i < set->num_groups; ++i)
	{
		set->group_mask[i] = 0;
	}
}

void wlibc_cpu_set(int cpu, cpu_set_t *set)
{
	if (set == NULL)
	{
		errno = EINVAL;
		return;
	}

	if (cpu < 0 || cpu >= set->num_cpus)
	{
		errno = EINVAL;
		return;
	}

	set->group_mask[cpu / 64] |= 1ull << (cpu % 64);
}

void wlibc_cpu_clr(int cpu, cpu_set_t *set)
{
	if (set == NULL)
	{
		errno = EINVAL;
		return;
	}

	if (cpu < 0 || cpu >= set->num_cpus)
	{
		errno = EINVAL;
		return;
	}

	set->group_mask[cpu / 64] &= ~(1ull << (cpu % 64));
}

int wlibc_cpu_isset(int cpu, cpu_set_t *set)
{
	if (set == NULL)
	{
		errno = EINVAL;
		return 0;
	}

	if (cpu < 0 || cpu >= set->num_cpus)
	{
		errno = EINVAL;
		return 0;
	}

	return ((set->group_mask[cpu / 64] & (1ull << (cpu % 64))) != 0);
}

int wlibc_cpu_count(cpu_set_t *set)
{

	if (set == NULL)
	{
		errno = EINVAL;
		return -1;
	}

	int count = 0;
	for (int i = 0; i < set->num_groups; ++i)
	{
		// `__popcnt64` can return a max value of 64.
		count += (int)__popcnt64(set->group_mask[i]);
	}

	return count;
}

void wlibc_cpu_and(cpu_set_t *destset, cpu_set_t *srcset1, cpu_set_t *srcset2)
{
	if (destset == NULL || srcset1 == NULL || srcset2 == NULL)
	{
		errno = EINVAL;
		return;
	}

	if (!(destset->num_groups == srcset1->num_groups && destset->num_groups == srcset2->num_groups))
	{
		errno = EINVAL;
		return;
	}

	for (int i = 0; i < destset->num_groups; ++i)
	{
		destset->group_mask[i] = srcset1->group_mask[i] & srcset2->group_mask[i];
	}
}

void wlibc_cpu_or(cpu_set_t *destset, cpu_set_t *srcset1, cpu_set_t *srcset2)
{
	if (destset == NULL || srcset1 == NULL || srcset2 == NULL)
	{
		errno = EINVAL;
		return;
	}

	if (!(destset->num_groups == srcset1->num_groups && destset->num_groups == srcset2->num_groups))
	{
		errno = EINVAL;
		return;
	}

	for (int i = 0; i < destset->num_groups; ++i)
	{
		destset->group_mask[i] = srcset1->group_mask[i] | srcset2->group_mask[i];
	}
}

void wlibc_cpu_xor(cpu_set_t *destset, cpu_set_t *srcset1, cpu_set_t *srcset2)
{
	if (destset == NULL || srcset1 == NULL || srcset2 == NULL)
	{
		errno = EINVAL;
		return;
	}

	if (!(destset->num_groups == srcset1->num_groups && destset->num_groups == srcset2->num_groups))
	{
		errno = EINVAL;
		return;
	}

	for (int i = 0; i < destset->num_groups; ++i)
	{
		destset->group_mask[i] = srcset1->group_mask[i] ^ srcset2->group_mask[i];
	}
}

int wlibc_cpu_equal(cpu_set_t *set1, cpu_set_t *set2)
{
	if (set1 == NULL || set2 == NULL)
	{
		errno = EINVAL;
		return 0;
	}

	if (set1->num_groups != set2->num_groups)
	{
		return 0;
	}

	if (set1->num_cpus != set2->num_cpus)
	{
		return 0;
	}

	for (int i = 0; i < set1->num_groups; ++i)
	{
		if (set1->group_mask[i] != set2->group_mask[i])
		{
			return 0;
		}
	}

	return 1;
}
