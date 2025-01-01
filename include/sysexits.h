/*
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_SYSEXITS_H
#define WLIBC_SYSEXITS_H

#define EX_OK          0  // Successful termination
#define EX_USAGE       64 // Incorrect command line usage
#define EX_DATAERR     65 // Data format error
#define EX_NOINPUT     66 // Cannot open input
#define EX_NOUSER      67 // User unknown
#define EX_NOHOST      68 // Host unknown
#define EX_UNAVAILABLE 69 // Service unavailable
#define EX_SOFTWARE    70 // Internal software error
#define EX_OSERR       71 // System error
#define EX_OSFILE      72 // Critical OS file missing
#define EX_CANTCREAT   73 // Cannot create output
#define EX_IOERR       74 // Input/Output error
#define EX_TEMPFAIL    75 // Temporary failure
#define EX_PROTOCOL    76 // Protocol error
#define EX_NOPERM      77 // Permission denied
#define EX_CONFIG      78 // Configuration error

#endif
