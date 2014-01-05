# Microsoft Developer Studio Project File - Name="XTunnel" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=XTunnel - Win32 IPv6 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "XTunnel.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "XTunnel.mak" CFG="XTunnel - Win32 IPv6 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "XTunnel - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "XTunnel - Win32 Release MinDependency" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "XTunnel - Win32 IPv6 Release MinDependency" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "XTunnel - Win32 IPv6 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "XTunnel - Win32 Debug"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /FR /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 Rasapi32.lib Iphlpapi.lib Ws2_32.lib newdev.lib setupapi.lib Shlwapi.lib Wininet.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /nodefaultlib
# Begin Custom Build - Performing registration
OutDir=.\Debug
TargetPath=.\Debug\XTunnel.dll
InputPath=.\Debug\XTunnel.dll
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "XTunnel - Win32 Release MinDependency"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ReleaseMinDependency"
# PROP BASE Intermediate_Dir "ReleaseMinDependency"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseMinDependency"
# PROP Intermediate_Dir "ReleaseMinDependency"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /D "_ATL_STATIC_REGISTRY" /FR /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 Rasapi32.lib Iphlpapi.lib Ws2_32.lib newdev.lib setupapi.lib Shlwapi.lib Wininet.lib /nologo /subsystem:windows /dll /machine:I386
# SUBTRACT LINK32 /debug
# Begin Custom Build - Performing registration
OutDir=.\ReleaseMinDependency
TargetPath=.\ReleaseMinDependency\XTunnel.dll
InputPath=.\ReleaseMinDependency\XTunnel.dll
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "XTunnel - Win32 IPv6 Release MinDependency"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "XTunnel___Win32_IPv6_Release_MinDependency"
# PROP BASE Intermediate_Dir "XTunnel___Win32_IPv6_Release_MinDependency"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "XTunnel___Win32_IPv6_Release_MinDependency"
# PROP Intermediate_Dir "XTunnel___Win32_IPv6_Release_MinDependency"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /D "_ATL_STATIC_REGISTRY" /FR /FD /c
# SUBTRACT BASE CPP /YX /Yc /Yu
# ADD CPP /nologo /MD /W3 /GX /O1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /D "_ATL_STATIC_REGISTRY" /D "SUPPORT_IPV6" /FR /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 Rasapi32.lib Iphlpapi.lib Ws2_32.lib newdev.lib setupapi.lib Shlwapi.lib Wininet.lib /nologo /subsystem:windows /dll /machine:I386
# SUBTRACT BASE LINK32 /debug
# ADD LINK32 Rasapi32.lib Iphlpapi.lib Ws2_32.lib newdev.lib setupapi.lib Shlwapi.lib Wininet.lib /nologo /subsystem:windows /dll /machine:I386
# SUBTRACT LINK32 /debug
# Begin Custom Build - Performing registration
OutDir=.\XTunnel___Win32_IPv6_Release_MinDependency
TargetPath=.\XTunnel___Win32_IPv6_Release_MinDependency\XTunnel.dll
InputPath=.\XTunnel___Win32_IPv6_Release_MinDependency\XTunnel.dll
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "XTunnel - Win32 IPv6 Debug"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "XTunnel___Win32_IPv6_Debug"
# PROP BASE Intermediate_Dir "XTunnel___Win32_IPv6_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "XTunnel___Win32_IPv6_Debug"
# PROP Intermediate_Dir "XTunnel___Win32_IPv6_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /FR /FD /GZ /c
# SUBTRACT BASE CPP /YX /Yc /Yu
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /D "SUPPORT_IPV6" /FR /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 Rasapi32.lib Iphlpapi.lib Ws2_32.lib newdev.lib setupapi.lib Shlwapi.lib Wininet.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# SUBTRACT BASE LINK32 /nodefaultlib
# ADD LINK32 Rasapi32.lib Iphlpapi.lib Ws2_32.lib newdev.lib setupapi.lib Shlwapi.lib Wininet.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /nodefaultlib
# Begin Custom Build - Performing registration
OutDir=.\XTunnel___Win32_IPv6_Debug
TargetPath=.\XTunnel___Win32_IPv6_Debug\XTunnel.dll
InputPath=.\XTunnel___Win32_IPv6_Debug\XTunnel.dll
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	
# End Custom Build

!ENDIF 

# Begin Target

# Name "XTunnel - Win32 Debug"
# Name "XTunnel - Win32 Release MinDependency"
# Name "XTunnel - Win32 IPv6 Release MinDependency"
# Name "XTunnel - Win32 IPv6 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\ProxyAuthDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\SetupDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\SSLApp.cpp
# End Source File
# Begin Source File

SOURCE=.\SSLSockWrap.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\TCPSockWrap.cpp
# End Source File
# Begin Source File

SOURCE=.\VPLaunch.cpp
# End Source File
# Begin Source File

SOURCE=.\VPSetup.cpp
# End Source File
# Begin Source File

SOURCE=.\XTunnel.cpp
# End Source File
# Begin Source File

SOURCE=.\XTunnel.def
# End Source File
# Begin Source File

SOURCE=.\XTunnel.idl
# ADD MTL /tlb ".\XTunnel.tlb" /h "XTunnel.h" /iid "XTunnel_i.c" /Oicf
# End Source File
# Begin Source File

SOURCE=.\XTunnel.rc
# End Source File
# Begin Source File

SOURCE=.\XTunnelCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\XTunnelManage.cpp
# End Source File
# Begin Source File

SOURCE=.\XTunnelStatus.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\SSLDrv\AccesInc.h
# End Source File
# Begin Source File

SOURCE=.\ProxyAuthDlg.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\SetupDlg.h
# End Source File
# Begin Source File

SOURCE=.\SSLApp.h
# End Source File
# Begin Source File

SOURCE=.\SSLSockWrap.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\VPLaunch.h
# End Source File
# Begin Source File

SOURCE=.\VPSetup.h
# End Source File
# Begin Source File

SOURCE=.\XTunnelCtrl.h
# End Source File
# Begin Source File

SOURCE=.\XTunnelManage.h
# End Source File
# Begin Source File

SOURCE=.\XTunnelStatus.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\Connect.Ico
# End Source File
# Begin Source File

SOURCE=.\MLERROR.ico
# End Source File
# Begin Source File

SOURCE=.\MLINFO.ico
# End Source File
# Begin Source File

SOURCE=.\multiicon.ico
# End Source File
# Begin Source File

SOURCE=.\VPLaunch.rgs
# End Source File
# Begin Source File

SOURCE=.\XTunnelCtrl.rgs
# End Source File
# End Group
# End Target
# End Project
