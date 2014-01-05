# Copyright (c) 2003-2005, Cavium Networks. All rights reserved.
#
# This Software is the property of Cavium Networks.  The Software and all
# accompanying documentation are copyrighted.  The Software made available
# here constitutes the proprietary information of Cavium Networks.  You
# agree to take reasonable steps to prevent the disclosure, unauthorized use
# or unauthorized distribution of the Software.  You shall use this Software
# solely with Cavium hardware.
#
# Except as expressly permitted in a separate Software License Agreement
# between You and Cavium Networks, you shall not modify, decompile,
# disassemble, extract, or otherwise reverse engineer this Software.  You
# shall not make any copy of the Software or its accompanying documentation,
# except for copying incident to the ordinary and intended use of the
# Software and the Underlying Program and except for the making of a single
# archival copy.
#
# This Software, including technical data, may be subject to U.S.  export
# control laws, including the U.S.  Export Administration Act and its
# associated regulations, and may be subject to export or import regulations
# in other countries.  You warrant that You will comply strictly in all
# respects with all such regulations and acknowledge that you have the
# responsibility to obtain licenses to export, re-export or import the
# Software.
#
# TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
# AND WITH ALL FAULTS AND CAVIUM MAKES NO PROMISES, REPRESENTATIONS OR
# WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
# TO THE SOFTWARE, INCLUDING ITS CONDITION, ITS CONFORMITY TO ANY
# REPRESENTATION OR DESCRIPTION, OR THE EXISTENCE OF ANY LATENT OR PATENT
# DEFECTS, AND CAVIUM SPECIFICALLY DISCLAIMS ALL IMPLIED (IF ANY) WARRANTIES
# OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR
# PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET
# POSSESSION OR CORRESPONDENCE TO DESCRIPTION.  THE ENTIRE RISK ARISING OUT
# OF USE OR PERFORMANCE OF THE SOFTWARE LIES WITH YOU.

ifndef OCTEON_ROOT
OCTEON_ROOT = ../..
endif

all: test-target application-target final

#  standard common Makefile fragment

include $(OCTEON_ROOT)/common.mk

#  include relevant component Makefile fragments

dir := $(OCTEON_ROOT)/executive
include $(dir)/cvmx.mk

dir := $(OCTEON_ROOT)/components/crypto-api/core/
include $(dir)/crypto.mk

# testcrypto tests all the algorithms supported by crypto library.
TARGET = testcrypto$(PREFIX)

COREMASK_APP=0x1

OBJS = $(OBJ_DIR)/testcrypto.o $(OBJ_DIR)/test-generic.o $(OBJ_DIR)/test_ipsec_api.o
CFLAGS_LOCAL = -g -O2 -W -Wall -Wno-unused-parameter
CFLAGS_LOCAL += -I$(OCTEON_ROOT)/target/include
CFLAGS_LOCAL += -DOCTEON_OPENSSL
CFLAGS_LOCAL += -DOCTEON_MODEL=$(OCTEON_MODEL)
CFLAGS_LOCAL += -DCOREMASK_APP=$(COREMASK_APP)

# Enable this to test Static Memory
# CFLAGS_LOCAL += -DOCTEON_OPENSSL_NO_DYNAMIC_MEMORY

# Enable this to test Crypto Generic APIs
# CFLAGS_LOCAL += -DTEST_GENERIC_APIS

# Enable this to test Crypto IPSec APIs
# CFLAGS_LOCAL += -DTEST_IPSEC_APIS

include $(OCTEON_ROOT)/application.mk

#
test-target:
ifndef  OCTEON_TARGET
	@echo "Usage:"
	@echo "     make OCTEON_TARGET=cvmx_64" 
	@echo "                or               "
	@echo "     make OCTEON_TARGET=linux_n32" 
	@echo "                or               "
	@echo "     make OCTEON_TARGET=linux_64" 
	@rm -rf $(TARGET) $(OBJ_DIR) $(CVMX_CONFIG) $(TARGET).stripped
	@exit 1
else
ifeq (cvmx_n32, $(OCTEON_TARGET))
	@echo "cvmx_n32 is not supported in this Crypto Release"
	@rm -rf $(TARGET) $(OBJ_DIR) $(CVMX_CONFIG) $(TARGET).stripped
	@exit 1 
endif
endif

final:
	cp -f $(TARGET) $(TARGET).stripped
	$(STRIP) $(TARGET).stripped

run: $(TARGET)
	oct-sim $(TARGET) -quiet -numcores=1 -noperf
	

clean:
	rm -rf $(TARGET) $(OBJ_DIR) $(CVMX_CONFIG) $(TARGET).stripped
