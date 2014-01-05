# Makefile
# Copyright (c) 2003-2006, Cavium Networks. All rights reserved.
#
# This Software is the property of Cavium Networks. The Software and all 
# accompanying documentation are copyrighted. The Software made available here 
# constitutes the proprietary information of Cavium Networks. You agree to take  
# reasonable steps to prevent the disclosure, unauthorized use or unauthorized 
# distribution of the Software. You shall use this Software solely with Cavium 
# hardware. 
#
# Except as expressly permitted in a separate Software License Agreement 
# between You and Cavium Networks, You shall not modify, decompile, 
# disassemble, extract, or otherwise reverse engineer this Software. You shall
# not make any copy of the Software or its accompanying documentation, except 
# for copying incident to the ordinary and intended use of the Software and 
# the Underlying Program and except for the making of a single archival copy.
#
# This Software, including technical data, may be subject to U.S. export 
# control laws, including the U.S. Export Administration Act and its 
# associated regulations, and may be subject to export or import regulations 
# in other countries. You warrant that You will comply strictly in all 
# respects with all such regulations and acknowledge that you have the 
# responsibility to obtain licenses to export, re-export or import the 
# Software.
#
# TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" AND 
# WITH ALL FAULTS AND CAVIUM MAKES NO PROMISES, REPRESENTATIONS OR WARRANTIES, 
# EITHER EXPRESS,IMPLIED, STATUTORY,OR OTHERWISE, WITH RESPECT TO THE SOFTWARE,
# INCLUDING ITS CONDITION,ITS CONFORMITY TO ANY REPRESENTATION OR DESCRIPTION, 
# OR THE EXISTENCE OF ANY LATENT OR PATENT DEFECTS, AND CAVIUM SPECIFICALLY 
# DISCLAIMS ALL IMPLIED (IF ANY) WARRANTIES OF TITLE, MERCHANTABILITY, 
# NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,LACK OF VIRUSES,ACCURACY OR
# COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO 
# DESCRIPTION. THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE 
# SOFTWARE LIES WITH YOU.

include $(OCTEON_ROOT)/common.mk

# OpenSSL 0.9.8 Install Path
OPENSSL_TOP_DIR		= $(OCTEON_ROOT)/linux/embedded_rootfs/build/openssl-0.9.8d

#Octeon Crypto Linux Kernel Module 
OCTEON_CRYPTO_DIR    = $(OCTEON_ROOT)/components/crypto-api/core/cryptolinux

#Library Name
LIBNAME=libocteon

INCLUDES		+= -I.

#OpenSSL 0.9.8 Includes
INCLUDES 		+= -I$(OPENSSL_TOP_DIR)/include

#CryptoLinux Includes
INCLUDES       += -I$(OCTEON_CRYPTO_DIR)

CFLAGS_LOCAL  += $(DEFINES) $(INCLUDES) -O3 -Wall

OBJ= e_octeon.o 

SHLIBNAME=$(LIBNAME).so

ifeq (,$(findstring linux,$(OCTEON_TARGET)))
   ${error Invalid value for OCTEON_TARGET. Supported values: linux_64, linux_n32}
else
   ifeq (linux_o32, $(OCTEON_TARGET))
      ${error Invalid value for OCTEON_TARGET. Supported values: linux_64, linux_n32}
   endif
endif

all: check engine

check:
	@if [ ! -d $(OPENSSL_TOP_DIR) ]; then \
		echo "OpenSSL Top Directory does not exist \n"; \
		exit 1; \
	fi
	@if [ ! -d $(OCTEON_CRYPTO_DIR) ]; then \
		echo "Octeon Crypto Directory does not exist \n"; \
		exit 1; \
	fi

%.o: %.c
	$(CC) $(CFLAGS_LOCAL) $(CFLAGS_GLOBAL) -MD -c -o $@ $<

engine:	 $(OBJ)
	$(CC) $(CFLAGS_GLOBAL) -shared -o $(SHLIBNAME) $(OBJ) -L$(OPENSSL_TOP_DIR) -lcrypto

clean:
	@rm -f *.o *.so


	

