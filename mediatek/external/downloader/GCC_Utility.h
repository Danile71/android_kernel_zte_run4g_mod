#ifndef	_GCC_UTILITY_H_
#define	_GCC_UTILITY_H_

#if defined(__GNUC__)
#include <stdio.h>

#include <stdint.h>
#include <stdbool.h>

#include <unistd.h>
#define Sleep(n) usleep((n)*1000)

//typedef uint32_t DWORD;
//typedef uint32_t* LPDWORD;
typedef unsigned long DWORD;
typedef unsigned long* LPDWORD;
typedef char* LPSTR;
typedef unsigned char* LPBYTE;
typedef void* LPVOID;
typedef const void* LPCVOID;

typedef uint16_t WORD;
typedef unsigned int   UINT;
typedef unsigned char  BYTE;

typedef int HANDLE;

#undef NULL
#define NULL 0

#ifdef __cpulsplus
#include <cstdarg>
#endif

typedef __builtin_va_list __gnuc_va_list;
typedef __gnuc_va_list va_list;

#define _vsnprintf vsnprintf
#define stricmp strcasecmp
#define strnicmp strncasecmp

typedef struct _SYSTEMTIME {
  WORD wYear;
  WORD wMonth;
  WORD wDayOfWeek;
  WORD wDay;
  WORD wHour;
  WORD wMinute;
  WORD wSecond;
  WORD wMilliseconds;
} SYSTEMTIME, *PSYSTEMTIME;

DWORD GetCurrentProcessId();

DWORD GetCurrentThreadId();

#define ULONG unsigned long
ULONG GetTickCount();
ULONG GetTickCount2();


void GetLocalTime(PSYSTEMTIME p_systime);
void OutputDebugString(const char* c);
bool WriteFile(HANDLE hFile, LPCVOID buffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, void* dummyforAPI);
bool ReadFile(HANDLE hFile, LPVOID buffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, void* dummyforAPI);
bool ChangeBaudRate(HANDLE hCOM, DWORD  baudrate);


FILE *_fsopen (const char *fname, const char *mode, int shflag);
char *_strupr( char *str );

#endif

#endif
