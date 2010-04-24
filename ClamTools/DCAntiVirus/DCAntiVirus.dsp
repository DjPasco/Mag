# Microsoft Developer Studio Project File - Name="DCAntiVirus" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=DCAntiVirus - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "DCAntiVirus.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "DCAntiVirus.mak" CFG="DCAntiVirus - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "DCAntiVirus - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "DCAntiVirus - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "DCAntiVirus - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "." /I "../" /I "../ClamInclude" /I "../../Detours/Detours/include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 DCSanner.lib detours.lib detoured.lib /nologo /subsystem:windows /machine:I386 /out:"../Bin/DCAntiVirus.exe" /libpath:"../Bin" /libpath:"../../Detours/Detours/lib"

!ELSEIF  "$(CFG)" == "DCAntiVirus - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "." /I "../" /I "../ClamInclude" /I "../../Detours/Detours/include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 DCSannerD.lib detours.lib detoured.lib /nologo /subsystem:windows /pdb:"../Bin/DCAntiVirusD.pdb" /debug /machine:I386 /out:"../Bin/DCAntiVirusD.exe" /pdbtype:sept /libpath:"../Bin" /libpath:"../../Detours/Detours/lib"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "DCAntiVirus - Win32 Release"
# Name "DCAntiVirus - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Scanner"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Scanner\Scanner.cpp
# End Source File
# Begin Source File

SOURCE=.\Scanner\Scanner.h
# End Source File
# End Group
# Begin Group "Utils"

# PROP Default_Filter ""
# End Group
# Begin Group "Hook"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Hook\Hook.cpp
# End Source File
# Begin Source File

SOURCE=.\Hook\Hook.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\DCAntiVirus.cpp
# End Source File
# Begin Source File

SOURCE=.\DCAntiVirus.h
# End Source File
# Begin Source File

SOURCE=.\DCAntiVirus.rc
# End Source File
# Begin Source File

SOURCE=.\DCAntiVirusDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\DCAntiVirusDlg.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\DCAntiVirus.ico
# End Source File
# Begin Source File

SOURCE=.\res\DCAntiVirus.rc2
# End Source File
# Begin Source File

SOURCE=.\res\shield.ico
# End Source File
# End Group
# End Target
# End Project
