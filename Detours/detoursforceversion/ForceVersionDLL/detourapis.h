#include <windows.h>
BOOL (WINAPI *pfuncGetVersionEx)(LPOSVERSIONINFO pOSVersionInfo) = GetVersionEx;
BOOL WINAPI ForceGetVersionEx(LPOSVERSIONINFO pOSVersionInfo);
