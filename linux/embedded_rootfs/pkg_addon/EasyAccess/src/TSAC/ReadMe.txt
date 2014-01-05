I. System Requirements for Build
================================
1. Windows 2000/XP operating system
2. Platform SDK
3. Need to install the following windows update for RDP 6.0:
	Update for Windows XP (KB925876)
	http://support.microsoft.com/?kbid=925876


II. Components
==============

Cavium TSAC Terminal services for windows consists of the following software which is described below.

1.XTSAC	
=======

This is the ActiveX control that is used to connect to the terminal server through AccessPoint 

	Root Dir:
	--------
	\TSAC\XTSAC

	How to Build?
	-------------
	1. This is a VC++ 6.0 project and can be compiled using the Dev Studio IDE to create the XTSAC.ocx control
        2. Open \TSAC\XTSAC\XTSAC.dsw in the VC++ 6.0 IDE and build it for 'Win32 Release' 
	 to create XTSAC.OCX

	Customer Deliverables:
        ----------------------
        1. XTSAC.OCX, MSRDP.OCX, XTSAC.inf (only for CAB distributions) as part of XTSAC.cab

	Manual Usage Procedure:
        -----------------------
        Launch the webpage that holds the control.(SampleTSAC.html)

III. Invoke Mechanisms
======================

TSAC in windows can be invoked through the ActiveX mechanism (for IE only)

1. ActiveX Direct Invocation
============================

Deliverables: 
-------------
1. XTSAC.cab
2. SampleTSAC.html

Contents of XTSAC.cab
---------------------
1. XTSAC.OCX
2. MSRDP.OCX
3.XTSAC.inf

How to create the CAB file
--------------------------

Executing MakeRelease.bat  creates the files that need to be transferred to the EasyAccess/www/htdocs dir of the server

        Root Dir:
        ---------
        \TSAC

        Usage Procedure:
        ----------------
        1. When this batch file is executed the following files are created in the \TSAC\Release folder
                a. XTSAC.cab
                
        2. These files need to be transferred to the EasyAccess/www/htdocs dir of the server

        Dependencies:
        -------------
        1. The following builds should happen before this batch file is executed
                a. Release build of XTSAC.ocx
        2. The certificate and key files should be present.
                a. ..\SignCode\MyCert.spc 
                b. ..\SignCode\MyKey.pvk
        3. The signcode.exe and cabarc.exe tools must present in one of the system defined paths.


	

	
















