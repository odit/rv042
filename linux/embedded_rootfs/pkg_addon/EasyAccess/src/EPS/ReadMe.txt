I. System Requirements for Build
================================
1. Windows XP operating system with SP2
2. Platform SDK for Windows XP SP2.
3. Visual Studio 6.0 with Service Pack6.

II. Components
==============

Cavium EndPointSecurity for windows consists of the following software which is described below.

1.EndPointSecurity 	
==================

This is the ActiveX control that is used to enhance AccessPoint more secured one. 

	Root Dir:
	--------
	\EPS\EndPointSecurity

	How to Build?
	-------------
	1. This is a VC++ 6.0 project and can be compiled using the Dev Studio IDE to create the EPS.dll control
	2. Open \EPS\EndPointSecurity\EPS.dsw in the VC++ 6.0 IDE and build it for 'Win32 Release' 
	 to create EPS.dll

	(Add paths for Platform SDK for windows XP SP2 in Tools->Options->Directories->Include files and 
        Tools->Options->Directories->Library files respectively in the IDE in the end)

	Customer Deliverables:
        ----------------------
        1.EPS.dll, EPS.inf (only for CAB distributions) as part of EPS.cab

	Manual Usage Procedure:
        -----------------------
        Launch the webpage that holds the control.(SampleEPS.html)

	

III. Invoke Mechanisms
======================

EPS in windows can be invoked through the ActiveX mechanism (for IE only)

1. ActiveX Direct Invocation
============================

Deliverables: 
-------------
1. EPS.cab
2. SampleEPS.html

Contents of EPS.cab
---------------------
1. EPS.dll
2. EPS.inf

How to create the CAB file
--------------------------

Executing MakeRelease.bat  creates the files that need to be transferred to the EasyAccess/www/htdocs dir of the server

        Root Dir:
        ---------
        \EPS

        Usage Procedure:
        ----------------
        1. When this batch file is executed the following files are created in the \EPS\Release folder
                a. EPS.cab
                
        2. These files need to be transferred to the EasyAccess/www/htdocs dir of the server

        Dependencies:
        -------------
        1. The following builds should happen before this batch file is executed
                a. Release build of EPS.dll
        2. The certificate and key files should be present.
                a. ..\SignCode\MyCert.spc 
                b. ..\SignCode\MyKey.pvk
        3. The signcode.exe and cabarc.exe tools must present in one of the system defined paths.


	
Note:
----

1. On any version changes modify the following places of the source code
	a. Output file concerned with EPS.dll.
	b. ChangLog.txt in the root folder.
	d. EPS.inf
	e. SampleEPS.html
	f. ReleaseNotes.txt in the root folder.
	(In server side - 1. cgi-bin\userpages\common\epslogin.c)
				



	
















