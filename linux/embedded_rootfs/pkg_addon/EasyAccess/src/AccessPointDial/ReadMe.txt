I. System Requirements for Build
================================
1. Windows 2000/XP operating system
2. Windows DDK
3. Platform SDK
4. Java SDK (v1.5)

II. Components
==============

MenloLogic AccessPoint Dial up adapter for windows consists of the following pieces of software as described below.

1. SSLDrv
===========

This is the Dial up adapter for windows SSL Tunneling.

        Root Dir:
        ---------
        \AccessPointDial\SSLDrv

        How to Build?
        -------------
        1. Open the 'Windows 2000 Free Build Environment' from the 'Build Environment' startup menu of DDK
        2. Use 'build -cZ' command to generate the SSLDrv.sys file
        3. The SSLDrv.txt file is present in the Inf Dir.
        4. Note that SSLDrv.inf has been renamed SSLDrv.txt to avoid the clash with XTunnel.inf

        Customer Deliverables:
        ----------------------
        1. SSLDrv.sys as part of XTunnel.cab/XTunnel6.cab/WindowsVPDialer.Jar/WindowsVPDialer6.Jar
        2. SSLDrv.txt as part of XTunnel.cab/XTunnel6.cab/WindowsVPDialer.Jar/WindowsVPDialer6.Jar

        Manual Install Procedure:(Win 2k)
        ---------------------------------
        
		ControlPanel->Add/Remove Hardware->Add/Troubleshoot a device->Add a new Device->No, I want to select The Hardware 
        from a list-> Network Adapters->HaveDisk->Browse->(Select SSLDrv.INF - SSLDrv.sys should also be in the 
        same folder as SSLDrv.inf)-> Next ->Next->Finish

        Create a phone book entry using the Menlo Logic SSL Adapter using the dafault settings for all values and name it 
        'VirtualPassage'.

        To Uninstall the driver go to ControlPanel->System->Device Manager->Network Adapters-> MenloLogic SSL Adapter->
        (right click)-> Uninstall.

Note: The driver is part of XTunnel.cab/XTunnel6.cab/WindowsVPDialer.Jar/WindowsVPDialer6.Jar and will be installed automatically

        
2. XTunnel
===========
 
This is the ActiveX control that is launched from the webpage/executable that is used to install the driver and connect to the
SSL Server. This also uninstalls the driver by invoking the uninstaller.

        Root Dir:
        ---------
        \AccessPointDial\XTunnel

        How to Build?
        -------------
        1. This is a VC++ 6.0 project and can be compiled using the Dev Studio IDE to create the XTunnel.dll control
        2. Add paths for newdev.h and newdev.lib in Tools->Options->Directories->Include files and 
        Tools->Options->Directories->Library files respectively in the IDE
        3. Open \AccessPointDial\XTunnel\XTunnel.dsw in the VC++ 6.0 IDE and build it for 'Win32 Release MinDependency' 
         (or 'Win32 IPv6 Release MinDependency' if IPv6 support is desired) to create XTunnel.dll
        
        Customer Deliverables:
        ----------------------
        1. XTunnel.dll, XTunnel.inf (only for CAB distributions) as part of XTunnel.cab/XTunnel6.cab/WindowsVPDialer.Jar//WindowsVPDialer6.Jar
		
        Manual Usage Procedure:
        -----------------------
        Launch the webpage that holds the control.(SampleConnect.html or SampleJar.html)(or SampleConnect6.html 
        or SampleJar6.html if IPv6 support is desired)


3. UninstallVTPassage 
======================
This is the uninstall application for the driver.

        Root Dir:
        ---------
        \AccessPointDial\UninstallVTPassage

        How to Build?
        -------------
        1. Open \AccessPointDial\Uninstall\UninstallVTPassage.dsw in VC++ 6.0 IDE compile it for 'Win32 Release' to create 
        UninstallVTPassage .exe

        Customer Deliverables:
        ----------------------
        1. UninstallVTPassage .exe as part of XTunnel.cab/XTunnel6.cab/WindowsVPDialer.Jar/WindowsVPDialer6.Jar


        Manual Usage Procedure:
        -----------------------
        1. Execute the UninstallVTPassage .exe- This uninstalls the driver, deletes the phone book entry named 'Virtual 
        Passage', deregisters the XTunnel.dll, deletes XTunnel.dll SSLDrv.sys SSLDrv.txt, deletes the folder where the Jar 
		file has been extracted (only for Jar distributions) and finally deletes itself 
        
Note: The exe is now part of XTunnel.cab/XTunnel6.cab/WindowsVPDialer.Jar/WindowsVPDialer6.Jar and is invoked automatically through a webpage.

4. VirtualPassageExe
======================
This is the launcher application for the XTunnel dll when used with the Java installer

        Root Dir:
        ---------
        \AccessPointDial\VirtualPassageExe

        How to Build?
        -------------
        1. Open \AccessPointDial\VirtualPassageExe\VirtualPassageExe.dsw in VC++ 6.0 IDE compile it for 'Win32 Release' 
        (or 'Win32 IPv6 Release' if IPv6 support is desired) to create VirtualPassageExe.exe

        Customer Deliverables:
        ----------------------
        1. VirtualPassageExe.exe as part of WindowsVPDialer.Jar/WindowsVPDialer6.Jar


        Manual Usage Procedure:
        ----------------------
        1. Execute the VirtualPassageExe.exe with proper command line arguments - See samplejar.html( or SampleJar6.html)
        for an example
        
Note: The exe is  part of WindowsVPDialer.Jar/WindowsVPDialer6.Jar and is invoked automatically through a webpage.

5. VPDesktopClientInstall
=========================
        This is the launcher application for the VPDesktopClient 
	
	Root Dir:
	--------
	\AccessPointDial\VPDesktopClineInstall

	How to build?
	------------
	1. Open \AccessPointDial\VPDesktopClientInstall\VPDesktopClientInstall.dsw in VC++ 6.0 IDE compile it for 'Win32 	Release' to create VPDesktopClientInstall.exe.
	
	Customer Deliverables:
        ----------------------
        1. VPDesktopClientInstall.exe as part of VPInstall.cab

	Manual Usage Procedure:
	-----------------------
	1.Execute the VPDesktopClientInstall.exe. It installs the VPDesktopClient.

6. VPDesktopClient
==================
	This is the Client which is used to run VP as standAlone.

	Root Dir:
	--------
	\AccessPointDial\VPDesktopClient

	How to build?
	------------
	1. Open \AccessPointDial\VPDesktopClient\VPDesktopClient.dsw in VC++ 6.0 IDE compile it for 'Win32 		Release' to create VPDesktopClient.exe.
	
	Customer Deliverables:
        ----------------------
        1. VPDesktopClientl.exe as part of VPInstall.cab

	Manual Usage Procedure:
	-----------------------
	1.Execute the VPDesktopClient.exe, Dialog pops up to enter input parameters. After getting authenticated VP wil be 		connected.																	                                                                           
III. Invoke Mechanisms
======================

Virtual Passage in windows can be invoked through the ActiveX mechanism (for IE only) or through the Java Installer
mechanism (For IE, Firefox, Mozilla etc)

1. Java Invocation
==================

Deliverables: 
-------------
1. WindowsVPInstaller.Jar 
2. WindowsVPDialer.Jar (or WindowsVPDialer6.Jar for IPv6 Support)
3. SampleJar.html (or SampleJar6.html for IPv6 Support)

Contents of WindowsVPInstaller.Jar
----------------------------------
1. VPInstaller.class
2. VPExtracter.class
3. VPDebug.class

Contents of WindowsVPDialer.Jar
-------------------------------
1. SSLDrv.sys
2. SSLDrv.txt
3. XTunnel.dll
4. UninstallVTPassage.exe
5. VirtualPassageExe.Exe

How to create the JAR files
---------------------------

Executing MakeJar.bat (or MakeJar6.bat for IPv6 Support) creates the files that need to be transferred to the 
EasyAccess/www/htdocs dir of the server

        Root Dir:
        ---------
        \AccessPointDial

        Usage Procedure:
        ----------------
        1. When this batch file is executed the following files are created in the \AccessPointDial\Release folder
                a. WindowsVPInstaller.jar
				b. WindowsVPDialer.jar (or WindowsVPDialer6.Jar)
                
        2. These files need to be transferred to the EasyAccess/www/htdocs dir of the server

        Dependencies:
        -------------
        1. The following builds should happen before this batch file is executed
                a. Free build of SSLDrv
                b. Release build of XTunnel.dll
                c. Release build of UninstallVTPassage 
				d. Release build of VirtualPassageExe
        2. The certificate file should be present.
                a. ..\SignCode\mycert.p12
           (Note: \AccessPointDial\HowTo_GenerateCertStore.txt has more information on creating *.p12 files from
		    *.spc and *.key files)
        3. The following tools should be present in one of the system defined paths
				a. javac
				b. jar
				c. jarsigner
		4. The environment variable %CLASSPATH% should have the complete paths of plugin.jar and rt.jar.
		

2. ActiveX Direct Invocation
============================

Deliverables: 
-------------
1. XTunnel.cab (or XTunnel6.cab)
2. SampleConnect.html (or SampleConnect6.html)

Contents of XTunnel.cab/XTunnel6.cab
------------------------------------
1. SSLDrv.sys
2. SSLDrv.txt
3. XTunnel.dll
4. XTunnel.inf
5. UninstallVTPassage .exe

How to create the CAB file
--------------------------

Executing MakeRelease.bat (or MakeRelease6.bat if IPv6 support is desired) creates the files that need to be transferred
to the EasyAccess/www/htdocs dir of the server

        Root Dir:
        ---------
        \AccessPointDial

        Usage Procedure:
        ----------------
        1. When this batch file is executed the following files are created in the \AccessPointDial\Release folder
                a. XTunnel.cab (or XTunnel6.cab)
                
        2. These files need to be transferred to the EasyAccess/www/htdocs dir of the server

        Dependencies:
        -------------
        1. The following builds should happen before this batch file is executed
                a. Free build of SSLDrv
                b. Release build of XTunnel.dll
                c. Release build of UninstallVTPassage 
        2. The certificate and key files should be present.
                a. ..\SignCode\MyCert.spc 
                b. ..\SignCode\MyKey.pvk
        3. The signcode.exe and cabarc.exe tools must present in one of the system defined paths.

3. VPDesktopClient
==================

Deliverables:
-------------
1.VPInstall.cab
2.SampleInstall.html

Contents of VPInstall.cab
-------------------------
1. SSLDrv.sys
2. SSLDrv.txt
3. XTunnel.dll
4. XTunnel.inf
5. UninstallVTPassage .exe
6. VPDesktopClient.exe
7. VPDesktopClientInstall.exe
8. VPDesktopClientInstall.inf
9. VPUnInstall.ico

How to create the CAB file
--------------------------
Executing MakeVPInstallRelease.bat creates the files that need to be transferred to the EasyAccess/www/htdocs dir of the server

	Root Dir:
	--------
	\AccessPointDial
	
	Usage Procedure:
        ----------------
        1. When this batch file is executed the following files are created in the \AccessPointDial\Release folder
                a. VPInstall.cab
                
        2. These files need to be transferred to the EasyAccess/www/htdocs dir of the server
	Dependencies:
        -------------
        1. The following builds should happen before this batch file is executed
		a. Free build of SSLDrv
                b. Release build of XTunnel.dll
                c. Release build of UninstallVTPassage 
		d. Release build of VPDesktopClient.exe
		e. Release build of VPDesktopClientInstall.exe
	2. The certificate and key files should be present.
                a. ..\SignCode\MyCert.spc 
                b. ..\SignCode\MyKey.pvk
	3. The signcode.exe and cabarc.exe tools must present in one of the system defined paths.

Note:
=====
1. On any version change modify it in the following places of the source code
        a. Change VER_SSLDRV_xx in VPSetup.h based on driver version
        b. Output file concerned - sys, exe, txt, inf AND XTunnel.dll
        c. ChangeLog.txt of SSLDrv or UninstallVTPassage or VirtualPassageExe or VPDesktopClientInstall or VPDesktopClient 	AND XTunnel
        d. XTunnel.inf
        e. SampleConnect.html and SampleConnect6.html
        f. ChangeLog.txt and ReleaseNotes.txt in the root folder
	g. VirtualPassageExe\Version.Info
2. WindowsVPDialer.jar & XTunnel.cab and WindowsVPDialer6.jar and XTunnel6.cab are present separately in the source code drop.
However the required version should be copied only as WindowsVPDialer.jar and XTunnel.cab into EasyAccess/www/htdocs. Makefiles 
do this automatically based on if IPv6 is selected in the build or not.
3. Limitations in VP with IPv6
	a. IP Address validation of input parameters not supported in v6
	b. Connect through Proxy Servers not supported in v6
	c. Addition of Default routes not supported in v6
	d. IPv6 not supported in windows 2k