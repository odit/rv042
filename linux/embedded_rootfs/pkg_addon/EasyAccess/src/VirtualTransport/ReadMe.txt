I. System Requirements for Build
================================
1. Windows 2000/XP operating system
2. Platform SDK

II. Components
==============

Virtual Transport windows consists of the following software which is described below.

1. MLVT
2. MLVTLsp
3. MLVTNsp
4. VirtualTransportUninstall

Note : All the above software can be build through VirtualTransport

	Root Dir:
	--------
	\VirtualTransport\src\VirtualTransport

	How to Build?
	-------------
	1. This is a VC++ 6.0 project and can be compiled using the Dev Studio IDE to create the MLVT.dll, MLVTLsp.dll, MLVTNsp.dll
 	and VirtualTransportUninstall.exe control
        2. Add paths for newdev.h and newdev.lib in Tools->Options->Directories->Include files and 
	Tools->Options->Directories->Library files respectively in the IDE
	3. Open \VirtualTransport\src\VirtualTransport\VirtualTransport.dsw in the VC++ 6.0 IDE and tick all the below files
	a. MLVT - win32 Release
	b. MLVTLsp - win32 Release
	c. MLVTNsp - win32 Release
	d. VirtualTransportUninstall - win32 Release in Build -> BatchBuild and then build it.

	Customer Deliverables:
        ----------------------
        1. MLVT.dll, MLVTLsp.dll, MLVTNsp.dll, SpOrder.dll, VirtualTransportUninstall.exe, MenloLSP.inf (only for CAB distributions) as part of MenloLSP.cab

	Manual Usage Procedure:
        -----------------------
        Launch the webpage that holds the control.(MLVirtualTransport.htm)

III. Invoke Mechanisms
======================

VirtualTransport in windows can be invoked through the ActiveX mechanism (for IE only)

1. ActiveX Direct Invocation
============================


Deliverables: 
-------------
1. MenloLSP.cab
2. MLVirtualTransport.htm

Contents of MenloLSP.cab
------------------------
1. MLVT.dll
2. MLVTLsp.dll 
3. MLVTNsp.dll
4. SpOrder.dll
5. VirtualTransportUninstall.exe
6. MenloLSP.inf 

How to create the CAB file
--------------------------

Executing MakeRelease.bat  creates the files that need to be transferred to the EasyAccess/www/htdocs dir of the server

        Root Dir:
        ---------
        \VirtualTransport\src\VirtualTransport\src\Bin

	Usage Procedure:
        ----------------
        1. When this batch file is executed the following files are created in the \VirtualTransport\Release folder
                a. MenloLSP.cab

        2. These files need to be transferred to the EasyAccess/www/htdocs dir of the server

	Dependencies:
        -------------
        1. The following builds should happen before this batch file is executed
                a. Release build of MLVT.dll
		b. Release build of MLVTLsp.dll 
		c. Release build of MLVTNsp.dll
		d. Release build of VirtualTransportUninstall.exe
        2. The certificate and key files should be present.
                a. ..\SignCode\MyCert.spc 
                b. ..\SignCode\MyKey.pvk
        3. The signcode.exe and cabarc.exe tools must present in one of the system defined paths.




	
	


	
	


