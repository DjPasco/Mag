/////////////////////////////////////////////////////////////////////////
// RemoteLib.h
//-----------------------------------------------------------------------
// Definition of exported functions for "RemoteLib.dll", which is a DLL
// that allows you to load/unload your own DLL into a remote running
// processes, AKA, code injection.
//
// This library provides API's for both Windows 9x plateform (including 
// 95/98/ME) and Windows NT plateform (including NT/2000/XP/2003).
//
// In case any function of this library fails, please call "GetLastError()"
// for extended error information. 
//
// This library is UNICODE compliant.
//
// This library is provided "as is" with no expressed or implied warranties.
// The author holds no responsibilities for any possible damages or loss of
// data that are caused by use of this library. The user must assume the
// entire risk of using this library.
//-----------------------------------------------------------------------
// Version History:
//
// Jan 03, 2005, 1.00 - Initial release.
// Jan 10, 2005, 1.01 - Changed RemoteGetModuleHandle so it does not expect absolute paths.
//-----------------------------------------------------------------------
// Author:
//
// Abin (abinn32@yahoo.com)
// Homepage: http://www.wxjindu.com/abin/
/////////////////////////////////////////////////////////////////////////

#ifndef __REMOTELIB_H__
#define __REMOTELIB_H__

#include <windows.h>

/////////////////////////////////////////////////////////////////////////
// UNICODE/ANSI Function Name Definition
/////////////////////////////////////////////////////////////////////////
#ifdef UNICODE // UNICODE is defined
#define RemoteLoadLibraryNT			RemoteLoadLibraryNTW
#define RemoteGetModuleFileName		RemoteGetModuleFileNameW
#define RemoteGetModuleHandleNT		RemoteGetModuleHandleNTW
#else // #ifdef UNICODE
#define RemoteLoadLibraryNT			RemoteLoadLibraryNTA
#define RemoteGetModuleFileName		RemoteGetModuleFileNameA
#define RemoteGetModuleHandleNT		RemoteGetModuleHandleNTA
#endif // #ifdef UNICODE

/////////////////////////////////////////////////////////////////////////
// Loading/Unloading DLLs (Windows NT/2000/XP/2003)
//-----------------------------------------------------------------------
// For NT Plateforms only.
// The remote process can be a window-less application.
/////////////////////////////////////////////////////////////////////////
HMODULE RemoteLoadLibraryNTA(DWORD dwTargetProcessID, LPCSTR lpszDllPath);
HMODULE RemoteLoadLibraryNTW(DWORD dwTargetProcessID, LPCWSTR lpszDllPath);
BOOL	RemoteFreeLibraryNT(DWORD dwTargetProcessID, HMODULE hModule);
HMODULE RemoteGetModuleHandleNTA(DWORD dwTargetProcessID, LPCSTR lpszDllPath);
HMODULE RemoteGetModuleHandleNTW(DWORD dwTargetProcessID, LPCWSTR lpszDllPath);
#endif // #ifndef __REMOTELIB_H__