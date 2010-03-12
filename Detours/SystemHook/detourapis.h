#include <windows.h>

HANDLE (WINAPI * pTrueCreateFileW)(LPCWSTR lpFileName,
								   DWORD dwDesiredAccess,
								   DWORD dwShareMode,
								   LPSECURITY_ATTRIBUTES lpSecurityAttributes,
								   DWORD dwCreationDisposition,
								   DWORD dwFlagsAndAttributes,
								   HANDLE hTemplateFile) = CreateFileW;

HANDLE WINAPI TransCreateFileW(LPCWSTR lpFileName,
							   DWORD dwDesiredAccess,
							   DWORD dwShareMode,
							   LPSECURITY_ATTRIBUTES lpSecurityAttributes,
							   DWORD dwCreationDisposition,
							   DWORD dwFlagsAndAttributes,
							   HANDLE hTemplateFile);

HANDLE (WINAPI *pTrueCreateFileA)(LPCSTR lpFileName,
								  DWORD dwDesiredAccess,
								  DWORD dwShareMode,
								  LPSECURITY_ATTRIBUTES lpSecurityAttributes,
								  DWORD dwCreationDisposition,
								  DWORD dwFlagsAndAttributes,
								  HANDLE hTemplateFile) = CreateFileA;

HANDLE WINAPI TransCreateFileA(LPCSTR lpFileName,
							   DWORD dwDesiredAccess,
							   DWORD dwShareMode,
							   LPSECURITY_ATTRIBUTES lpSecurityAttributes,
							   DWORD dwCreationDisposition,
							   DWORD dwFlagsAndAttributes,
							   HANDLE hTemplateFile);
