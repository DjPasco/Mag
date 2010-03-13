#pragma once

#define WINVER 0x0501

#ifndef VC_EXTRALEAN
	#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions

#ifndef _AFX_NO_AFXCMN_SUPPORT
	#include <afxcmn.h>                     // MFC support for Windows Common Controls
#endif 


#include <afxsock.h>            // MFC socket extensions









