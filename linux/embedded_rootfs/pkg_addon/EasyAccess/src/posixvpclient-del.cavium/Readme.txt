This manual describes the procedure for building Linux & Mac VPClient.

Package Dependecies:

OpenSSL:        OpenSSL-0.9.7f (Only static libraries are to be used)
Java:			1.4.2_12

            *************************************

How to create Certificate store for signing Java Code from (.spc) and (.pvk) files
__________________________________________________________________________________

Step 1:
	On a Windows2000 machine install the tool "pvkimprt".
	
Step 2:
	Import the certificate into a known location using pvkimprt <.spc filename> <.pvk filename>.

Step 3:
	Start->Run->mmc
	On the main window click "Console->Add/Remove Snap-in".Choose Certificates and select the 
	certificate imported in Step 2.

Step 4:
	Right click on the selected certificate and chose "properties" and give a friendly name that 
	will server as our <alias>

Step 5:
	From the Actions drop down select Export and export the certificate imported in Step 2.The 
	exported file will get saved as .pfx file.
	
Step 6:
	Import the .pfx file from Mozilla/Firefox browser through Edit->Preferences->Advanced->
	Certificates->Manage Certificates into "Your Certificates".This would import the contents of 
	the .pfx file into the Browser's certificate store.
	
Step 7:
	Export the certificate imported in Step 7, by clicking on "Backup" and store it inside 
	"posixvpclient".This file should have a .p12 extension.
	
	This is the certificate store that is to be used for signing Linux and Mac OS X VPClient.


How to build Mac OS X VPClient for the build platform
______________________________________________________

Requirements:

Processor:-	Intel or PPC
OS Version:
	Intel machines:- Mac OS X Tiger 10.4.x
	PPC machines:- Mac OS X Panther 10.3.x or Mac OS X Tiger 10.4.x

Tools:
	Intel machines:- gcc-4.0
	PPC machines:- gcc-3.3

Step 1:
	IPv4 :-
	Issue 'make -f Makefile.macosx' with the p.w.d as "posixvpclient".This will build all the 
	files under this package tree and generate "SignedMacVPClient.jar" and "SignedInstaller.jar" 
	in the folder  "release".
	
	IPv6 :-
	Issue 'make -f Makefile.macosx IPV6=true' with the p.w.d as "posixvpclient".This will build all
	the files under this package tree and generate "SignedMacVPClient6.jar"	in the folder 
	"release".

Step 2:
	Copy SignedInstaller.jar, SignedInstaller6.jar, SignedMacVPClient.jar and 
	SignedMacVPClient6.jar from the Mac OS X machine to the corresponding folder in the Linux 
	machine.


Note:
	VPClient compiled using the above instructions will run only on the platform on which it was 
	compiled, i.e.,
	
	1.VPClient compiled on an Intel machine will run only on Intel and not on PPC machines.
	2.Similarly VPClient compiled on a PPC machine will run only on PPC and not on Intel machines. 



How to cross-compile Mac OS X VPClient for PPC machines
_______________________________________________________

Requirements:

Processor:-	Intel
OS Version:- Mac OS X Tiger 10.4.x
Tools:- gcc-3.3

Step 1:
	IPv4 :-
	Issue 'make -f Makefile.macosx ppc' with the p.w.d as "posixvpclient".This will build all 
	the files under this package tree and generate "SignedMacVPClient.jar" and
	"SignedInstaller.jar" in the folder  "release".

	IPv6 :-
	Issue 'make -f Makefile.macosx ppc IPV6=true' with the p.w.d as "posixvpclient".This will 
	build all the files under this package tree and generate "SignedMacVPClient6.jar" in the 
	folder "release".

Step 2:
	Copy SignedInstaller.jar, SignedInstaller6.jar, SignedMacVPClient.jar and 
	SignedMacVPClient6.jar from the Mac OS X machine to the corresponding folder in the Linux 
	machine.


Note:
	VPClient compiled using the above instructions will run only on PPC and not on Intel machines.



How to cross-compile Mac OS X VPClient for Intel machines
_________________________________________________________

Requirements:

Processor:-	PPC
OS Version:- Mac OS X Tiger 10.4.x
Tools:- gcc-4.0, MacOSX10.4u.sdk

Step 1:
	IPv4 :-
	Issue 'make -f Makefile.macosx intel' with the p.w.d as "posixvpclient".This will build all 
	the files under this package tree and generate "SignedMacVPClient.jar" and
	"SignedInstaller.jar" in the folder  "release".

	IPv6 :-
	Issue 'make -f Makefile.macosx intel IPV6=true' with the p.w.d as "posixvpclient".This will 
	build all the files under this package tree and generate "SignedMacVPClient6.jar" in the 
	folder "release".

Step 2:
	Copy SignedInstaller.jar, SignedInstaller6.jar, SignedMacVPClient.jar and 
	SignedMacVPClient6.jar from the Mac OS X machine to the corresponding folder in the Linux 
	machine.

Note:
	VPClient compiled using the above instructions will run only on Intel and not on PPC machines.



How to build universal Mac OS X VPClient 
________________________________________

Requirements:

Processor:-	Intel or PPC
OS Version:- Mac OS X Tiger 10.4.x
Tools:- gcc-3.3, gcc-4.0, MacOSX10.4u.sdk

Step 1:
	IPv4 :-
	Issue 'make -f Makefile.macosx universal' with the p.w.d as "posixvpclient".This will build 
	all the files under this package tree and generate "SignedMacVPClient.jar" and
	"SignedInstaller.jar" in the folder  "release".

	IPv6 :-
	Issue 'make -f Makefile.macosx universal IPV6=true' with the p.w.d as "posixvpclient".This 
	will build all the files under this package tree and generate "SignedMacVPClient6.jar" in the 
	folder "release".

Step 3:
	Copy SignedInstaller.jar, SignedInstaller6.jar, SignedMacVPClient.jar and 
	SignedMacVPClient6.jar from the Mac OS X machine to the corresponding folder in the Linux 
	machine.


Note:
	1.VPClient may be compiled from either a PPC or an Intel machine.
	2.VPClient will run on both PPC and Intel machines.



How to build Linux VPClient
___________________________

Step 1:
	The OpenSSL static libraries should be generated first.Instructions for the same could be 
	found in "openssl/Readme.txt" .
	
Step 2:
	IPv4 :-
	Issue 'make -f Makefile.linux' with the p.w.d as "posixvpclient".This will build all the files 
	under this package tree and generate "SignedVPClient.jar" and "SignedInstaller.jar" in the 
	folder  "release".
	
	IPv6 :-
	Issue 'make -f Makefile.linux IPV6=true' with the p.w.d as "posixvpclient".This will build all
	the files under this package tree and generate "SignedVPClient6.jar" and "SignedInstaller6.jar"
	in the folder "release".
	
Step 3:
	Place the "posixvpclient" folder inside ...../EasyAccess/src/ . Check out the Makefiles that 
	need to have posixvpclient in their dependency list.

Step 4:
	Issue the command 'make install' with p.w.d as "EasyAccess" and the jar files generated	will 
	get copied into '...../EasyAccess/www/htdocs'.
	

Note: Whenever the version changes, make sure the same has been updated in java/version.info.
	
		*************************************
