#ifndef PHYX_OLD_MINGW_COMPAT_MINWINDEF_H
#define PHYX_OLD_MINGW_COMPAT_MINWINDEF_H

/*
 * raylib 5.5 includes <minwindef.h> in its Windows clipboard helper.
 * MinGW.org's legacy w32api headers do not ship that split SDK header.
 *
 * Do not include MinGW.org's <windef.h> here: it also declares HWND, while
 * raylib forward-declares HWND before including the clipboard implementation.
 */
#include <stddef.h>

#ifndef __declspec
#define __declspec(e) __attribute__((e))
#endif

#ifndef DECLSPEC_IMPORT
#define DECLSPEC_IMPORT __declspec(dllimport)
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef NULL
#ifdef __cplusplus
#define NULL 0
#else
#define NULL ((void *)0)
#endif
#endif

#ifndef PHYX_OLD_MINGW_COMPAT_BASE_TYPES
#define PHYX_OLD_MINGW_COMPAT_BASE_TYPES
typedef int BOOL;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int UINT;
typedef unsigned long DWORD;
typedef long LONG;
typedef void VOID;
typedef void *LPVOID;
typedef const char *LPCSTR;
typedef char *LPSTR;
typedef const wchar_t *LPCWSTR;
typedef wchar_t *LPWSTR;
typedef size_t SIZE_T;
#endif

#endif
