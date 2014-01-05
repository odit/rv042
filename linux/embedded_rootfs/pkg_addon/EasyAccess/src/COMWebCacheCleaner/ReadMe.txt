I. System Requirements for Build
================================
1. Windows 2000/XP operating system
2. Platform SDK

II. Components
==============

Cavium COMWebCacheCleaner for windows consists of the following software which is described below.

1.MLWebCacheClener
------------------

This is the Activex Control that is used to cleanup all the files respect to AccessPoint

	
	Root Dir:
	--------
	\COMWebCacheCleaner\MLWebCacheCleaner

	How to Build?
	-------------
	1. This is a VC++ 6.0 project and can be compiled using the Dev Studio IDE to create the MLWebCacheCleaner.dll control
        2. Open \COMWebCacheCleaner\MLWebCacheCleaner\MLWebCacheCleaner.dsw in the VC++ 6.0 IDE and build it for 'Win32 Unicode Release MinDependency' 
	 to create MLWebCacheCleaner.dll

	Customer Deliverables:
        ----------------------
        1. MLWebCacheCleaner.dll, MLWebCacheCleaner.inf (only for CAB distributions) as part of MLWebCacheCleaner.cab

	Manual Usage Procedure:
        -----------------------
        Launch the webpage that holds the control.(WebCacheCleaner.html)


III. Invoke Mechanisms
======================
COMWebCacheCleaner in windows can be invoked through the ActiveX mechanism (for IE only)

1. ActiveX Direct Invocation
============================

Deliverables: 
-------------
1. MLWebCacheCleaner.cab
2. WebCacheCleaner.html

Contents of XTSAC.cab
---------------------
1. MLWebCacheCleaner.dll
2. MLWebCacheCleaner.inf

How to create the CAB file
--------------------------

Executing MakeRelease.bat  creates the files that need to be transferred to the EasyAccess/www/htdocs dir of the server

        Root Dir:
        ---------
        \COMWebCacheCleaner

	Usage Procedure:
        ----------------
        1. When this batch file is executed the following files are created in the \COMWebCacheCleaner\Release folder
                a. MLWebCacheCleaner.cab
                
        2. These files need to be transferred to the EasyAccess/www/htdocs dir of the server

	Dependencies:
        -------------
        1. The following builds should happen before this batch file is executed
                a. Unicode Release build of MLWebCacheCleaner.dll
        2. The certificate and key files should be present.
                a. ..\SignCode\MyCert.spc 
                b. ..\SignCode\MyKey.pvk
        3. The signcode.exe and cabarc.exe tools must present in one of the system defined paths.




