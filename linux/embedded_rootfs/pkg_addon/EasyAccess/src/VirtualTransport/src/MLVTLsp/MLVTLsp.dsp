# Microsoft Developer Studio Project File - Name="MLVTLsp" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=MLVTLsp - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "MLVTLsp.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "MLVTLsp.mak" CFG="MLVTLsp - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "MLVTLsp - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "MLVTLsp - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/MenloLogic/MLVT", LLAAAAAA"
# PROP Scc_LocalPath "..\mlvt"
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "MLVTLsp - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MLVTLSP_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MLVTLSP_EXPORTS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 Gdi32.lib User32.lib Advapi32.lib ws2_32.lib Shlwapi.lib Psapi.lib Version.lib shell32.lib /nologo /dll /machine:I386 /out:"..\Bin/MLVTLsp.dll"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=if exist ..\bin\MLVTLsp.ilk  del ..\bin\MLVTLsp.ilk
# End Special Build Tool

!ELSEIF  "$(CFG)" == "MLVTLsp - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MLVTLSP_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MLVTLSP_EXPORTS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 crypt32.lib shell32.lib Gdi32.lib User32.lib Advapi32.lib ws2_32.lib Shlwapi.lib Psapi.lib Version.lib /nologo /dll /debug /machine:I386 /out:"..\Bin/MLVTLsp.dll" /pdbtype:sept
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=if exist ..\bin\MLVTLsp.ilk  del ..\bin\MLVTLsp.ilk
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "MLVTLsp - Win32 Release"
# Name "MLVTLsp - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\Include\MenloUtils.cpp
# End Source File
# Begin Source File

SOURCE=.\MLAsyncselect.cpp
# End Source File
# Begin Source File

SOURCE=.\MLDefaultAlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MLExtension.cpp
# End Source File
# Begin Source File

SOURCE=.\MLFtpAlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MLOverlap.cpp
# End Source File
# Begin Source File

SOURCE=.\MLSockInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\MLspi.cpp
# End Source File
# Begin Source File

SOURCE=.\MLSslAlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MLVTLsp.def
# End Source File
# Begin Source File

SOURCE=.\MLVTLsp.rc
# End Source File
# Begin Source File

SOURCE=.\SSLSockWrap.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\MLdef.h
# End Source File
# Begin Source File

SOURCE=.\MLDefaultAlg.h
# End Source File
# Begin Source File

SOURCE=.\MLFtpAlg.h
# End Source File
# Begin Source File

SOURCE=.\MLSslAlg.h
# End Source File
# Begin Source File

SOURCE=.\SSLSockWrap.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
