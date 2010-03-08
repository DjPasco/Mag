#if !defined(_STDAFX_H_)
#define _STDAFX_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//
// Exclude rarely-used stuff from Windows headers
//
#define VC_EXTRALEAN		

#include "..\Common\Common.h"

#ifdef UNICODE
	#pragma comment(linker, "/ENTRY:wWinMainCRTStartup")
#else
	#pragma comment(linker, "/ENTRY:WinMainCRTStartup")
#endif // #ifdef UNICODE

#include <afxwin.h>         // MFC core and standard components
#include <winsock.h>

#endif