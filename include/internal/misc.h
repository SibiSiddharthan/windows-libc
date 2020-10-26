#ifndef WLIBC_MB_WC_CONVERT_H
#define WLIBC_MB_WC_CONVERT_H

#include <wchar.h>
#include <string.h>
#include <stdlib.h>

char *wc_to_mb(const wchar_t *wstr);
wchar_t *mb_to_wc(const char *str);

char *mbstrcat(const char *str1, const char *str2);
wchar_t *wcstrcat(const wchar_t *wstr1, const wchar_t *wstr2);

int is_absolute_path(const char *str);
int is_absolute_pathw(const wchar_t *wstr);

void fs_to_bs(char *path);
void wfs_to_bs(wchar_t *wpath);

#endif