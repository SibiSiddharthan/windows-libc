/*
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
*/

#ifndef WLIBC_ERRNO_H
#define WLIBC_ERRNO_H

#include <wlibc.h>

_ACRTIMP int *__cdecl _errno(void);
_ACRTIMP errno_t __cdecl _set_errno(int error);
_ACRTIMP errno_t __cdecl _get_errno(int *error);
#define errno (*_errno())

_ACRTIMP unsigned long *__cdecl __doserrno(void);
_ACRTIMP errno_t __cdecl _set_doserrno(unsigned long error);
_ACRTIMP errno_t __cdecl _get_doserrno(unsigned long *error);
#define _doserrno (*__doserrno())

// GNU extensions
extern char *program_invocation_name;
extern char *program_invocation_short_name;

// Standard error codes
#define EPERM        1
#define ENOENT       2
#define ESRCH        3
#define EINTR        4
#define EIO          5
#define ENXIO        6
#define E2BIG        7
#define ENOEXEC      8
#define EBADF        9
#define ECHILD       10
#define EAGAIN       11
#define ENOMEM       12
#define EACCES       13
#define EFAULT       14
#define ENOTBLK      15
#define EBUSY        16
#define EEXIST       17
#define EXDEV        18
#define ENODEV       19
#define ENOTDIR      20
#define EISDIR       21
#define EINVAL       22
#define ENFILE       23
#define EMFILE       24
#define ENOTTY       25
#define EFBIG        27
#define ENOSPC       28
#define ESPIPE       29
#define EROFS        30
#define EMLINK       31
#define EPIPE        32
#define EDOM         33
#define ERANGE       34
#define EDEADLK      36
#define ENAMETOOLONG 38
#define ENOLCK       39
#define ENOSYS       40
#define ENOTEMPTY    41
#define EILSEQ       42

// MSVC string truncate
#define STRUNCATE 80

// Extra error codes
#define EBADRQC         56
#define EBADSLT         57
#define EBFONT          59
#define ENONET          64
#define ENOPKG          65
#define EREMOTE         66
#define EADV            68
#define ESRMNT          69
#define ECOMM           70
#define EMULTIHOP       72
#define EDOTDOT         73
#define ENOTUNIQ        74
#define EBADFD          75
#define ESTALE          76
#define EREMCHG         77
#define ELIBACC         82
#define ELIBBAD         83
#define ELIBSCN         84
#define ELIBMAX         85
#define ELIBEXEC        86
#define ERESTART        87
#define ESTRPIPE        88
#define EUSERS          89
#define EISNAM          90
#define EREMOTEIO       91
#define EDQUOT          92
#define EHOSTDOWN       93
#define ESHUTDOWN       94
#define ESOCKTNOSUPPORT 95
#define EPFNOSUPPORT    96
#define ETOOMANYREFS    97
#define ENOMEDIUM       98
#define EMEDIUMTYPE     99

// POSIX supplement (same as MSVC)
#define EADDRINUSE      100
#define EADDRNOTAVAIL   101
#define EAFNOSUPPORT    102
#define EALREADY        103
#define EBADMSG         104
#define ECANCELED       105
#define ECONNABORTED    106
#define ECONNREFUSED    107
#define ECONNRESET      108
#define EDESTADDRREQ    109
#define EHOSTUNREACH    110
#define EIDRM           111
#define EINPROGRESS     112
#define EISCONN         113
#define ELOOP           114
#define EMSGSIZE        115
#define ENETDOWN        116
#define ENETRESET       117
#define ENETUNREACH     118
#define ENOBUFS         119
#define ENODATA         120
#define ENOLINK         121
#define ENOMSG          122
#define ENOPROTOOPT     123
#define ENOSR           124
#define ENOSTR          125
#define ENOTCONN        126
#define ENOTRECOVERABLE 127
#define ENOTSOCK        128
#define ENOTSUP         129
#define EOPNOTSUPP      130
#define EOTHER          131
#define EOVERFLOW       132
#define EOWNERDEAD      133
#define EPROTO          134
#define EPROTONOSUPPORT 135
#define EPROTOTYPE      136
#define ETIME           137
#define ETIMEDOUT       138
#define ETXTBSY         139

#define EWOULDBLOCK_COMPAT 140

#define EWOULDBLOCK EAGAIN
#define EDEADLOCK   EDEADLK

#endif
