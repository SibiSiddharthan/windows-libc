#ifndef WLIBC_MB_WC_CONVERT_H
#define WLIBC_MB_WC_CONVERT_H

#include <wchar.h>
#include <string.h>
#include <stdlib.h>

char *wc_to_mb(const wchar_t *wstr);
wchar_t *mb_to_wc(const char *str);

void fs_to_bs(char *path);
void wfs_to_bs(wchar_t *wpath);

#endif