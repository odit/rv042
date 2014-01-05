Cavium Networks, OpenSSL specifications for Mac OS X & Linux VPClient.
######################################################################

This document describes about building the OpenSSL library for 
use with Mac OS X & Linux VPClient

Package Dependecies:

OpenSSL:        OpenSSL-0.97f


How to Compile OpenSSL on Linux
_______________________________

Step 1:
	Download the OpenSSL version 0.9.7f available at http://www.openssl.org/source/
	
Step 2:
	Extract it into a known directory say "/openssl-0.9.7f" and copy 
	"posixvpclient/openssl/Makefile_openssl_linux" into "/openssl-0.9.7f/Makefile" .
	
Step 3:
	Build the static libraries libssl.a and libcrypto.a by issuing the commands 'make' and 'make 
	install'
	
Step 4:
	A new folder /OpenSSL would have been created and the static libraries can be found inside 
	/OpenSSL/lib
	
Step 5:
	To ensure only our libraries are used at the time of linking, issue the following commands 
	with the p.w.d as "posixvpclient" ...

		cp /OpenSSL/lib/libssl.a openssl/linux_vp_ssl.a
		cp /OpenSSL/lib/libcrypto.a openssl/linux_vp_crypto.a
 
	

How to Compile OpenSSL on Mac OS X
__________________________________

Step 1:
	Download the OpenSSL version 0.9.7f available at http://www.openssl.org/source/
	
Step 2:
	Extract it into a known directory say "/openssl-0.9.7f" and copy 
	"posixvpclient/openssl/Makefile_openssl_macosx" into "/openssl-0.9.7f/Makefile" .
	
Step 3:
	Build the static libraries libssl.a and libcrypto.a by issuing the commands 'make' and 'make 
	install'
	
Step 4:
	A new folder /OpenSSL would have been created and the static libraries can be found inside 
	/OpenSSL/lib
	
Step 5:
	To ensure only our libraries are used at the time of linking, issue the following commands 
	with the p.w.d as "posixvpclient" ...
	
		libtool -static /OpenSSL/lib/libssl.a -o openssl/mac_vp_ssl.a 
		libtool -static /OpenSSL/lib/libcrypto.a -o openssl/mac_vp_crypto.a 
		

			 *************************************
