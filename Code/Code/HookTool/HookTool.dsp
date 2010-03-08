# Microsoft Developer Studio Project File - Name="HookTool" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=HookTool - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "HookTool.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "HookTool.mak" CFG="HookTool - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "HookTool - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "HookTool - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "HookTool - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "HOOKTOOL_EXPORTS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "NDEBUG" /D "_MBCS" /D "WIN32" /D "_WINDOWS" /D "HOOKTOOL_EXPORTS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386 /out:"..\..\Output\Hook.dll"

!ELSEIF  "$(CFG)" == "HookTool - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "HOOKTOOL_EXPORTS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GR /GX /ZI /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "HOOKTOOL_EXPORTS" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"..\..\Output\Hook.dll" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "HookTool - Win32 Release"
# Name "HookTool - Win32 Debug"
# Begin Source File

SOURCE=.\ApiHook.cpp
# End Source File
# Begin Source File

SOURCE=.\ApiHook.h
# End Source File
# Begin Source File

SOURCE=..\Common\Common.h
# End Source File
# Begin Source File

SOURCE=.\HookTool.cpp
# End Source File
# Begin Source File

SOURCE=.\HookTool.def
# End Source File
# Begin Source File

SOURCE=..\Common\IniFile.cpp
# End Source File
# Begin Source File

SOURCE=..\Common\IniFile.h
# End Source File
# Begin Source File

SOURCE=.\Injector.cpp
# End Source File
# Begin Source File

SOURCE=.\Injector.h
# End Source File
# Begin Source File

SOURCE=.\Interlocked.h
# End Source File
# Begin Source File

SOURCE=..\Common\LockMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\Common\LockMgr.h
# End Source File
# Begin Source File

SOURCE=..\Common\LogFile.h
# End Source File
# Begin Source File

SOURCE=..\Common\ModuleInstance.cpp
# End Source File
# Begin Source File

SOURCE=..\Common\ModuleInstance.h
# End Source File
# Begin Source File

SOURCE=.\ModuleScope.cpp
# End Source File
# Begin Source File

SOURCE=.\ModuleScope.h
# End Source File
# Begin Source File

SOURCE=.\NtDriverController.cpp
# End Source File
# Begin Source File

SOURCE=.\NtDriverController.h
# End Source File
# Begin Source File

SOURCE=.\NtInjectorThread.cpp
# End Source File
# Begin Source File

SOURCE=.\NtInjectorThread.h
# End Source File
# Begin Source File

SOURCE=.\NtProcessMonitor.cpp
# End Source File
# Begin Source File

SOURCE=.\NtProcessMonitor.h
# End Source File
# Begin Source File

SOURCE=..\Common\SysUtils.h
# End Source File
# End Target
# End Project
