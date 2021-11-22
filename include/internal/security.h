/*
   Copyright (c) 2020-2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef SECURITY_INTERNAL_H
#define SECURITY_INTERNAL_H

#include <winnt.h>
#include <sys/types.h>

void initialize_sids();
void cleanup_security_decsriptors();

PISECURITY_DESCRIPTOR_RELATIVE get_security_descriptor(mode_t mode, int is_directory);

#endif
