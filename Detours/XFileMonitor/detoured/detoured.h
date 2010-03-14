//////////////////////////////////////////////////////////////////////////////
//
//  Presence of this DLL (detoured.dll) marks a process as detoured.
//
//  Microsoft Research Detours Package, Version 2.1.
//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//

#ifdef DETOURED_EXPORTS
#define DETOURED_API __declspec(dllexport)
#else
#define DETOURED_API __declspec(dllimport)
#endif

HMODULE DETOURED_API WINAPI Detoured();

//
///////////////////////////////////////////////////////////////// End of File.
