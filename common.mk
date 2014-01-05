# Copyright (c) 2005, Cavium Networks. All rights reserved.
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

#
#  common Makefile fragment
#
#  $Id: common.mk,v 1.1.1.1 2007-04-05 08:46:00 tt Exp $ $Name: not supported by cvs2svn $
#

#  Octeon tools

#  Octeon default flags
CPPFLAGS_GLOBAL = -I$(OCTEON_ROOT)/target/include -Iconfig
CPPFLAGS_GLOBAL += $(OCTEON_CPPFLAGS_GLOBAL_ADD)

CFLAGS_GLOBAL = $(CPPFLAGS_GLOBAL)
CFLAGS_GLOBAL += $(OCTEON_CFLAGS_GLOBAL_ADD) 

ASFLAGS_GLOBAL = $(CPPFLAGS_GLOBAL)
ASFLAGS_GLOBAL += $(OCTEON_ASFLAGS_GLOBAL_ADD)

LDFLAGS_GLOBAL =
LDFLAGS_GLOBAL += $(LDFLAGS_GLOBAL_ADD)

LDFLAGS_PATH = -L$(OCTEON_ROOT)/target/lib

include $(OCTEON_ROOT)/common-config.mk
# Add flags set by common-config.mk
CFLAGS_GLOBAL += $(CFLAGS_COMMON_CONFIG)
ASFLAGS_GLOBAL += $(CFLAGS_COMMON_CONFIG)

ifndef OCTEON_TARGET
  OCTEON_TARGET=cvmx_64
endif

SUPPORTED_TARGETS=linux_64 linux_n32 linux_uclibc linux_o32 cvmx_n32 cvmx_64
MATCHED_TARGET=${findstring ${OCTEON_TARGET}, ${SUPPORTED_TARGETS}}

ifneq (${MATCHED_TARGET}, ${OCTEON_TARGET})
    ${error Invalid value for OCTEON_TARGET. Supported values: ${SUPPORTED_TARGETS}}
endif

ifeq (${OCTEON_TARGET},linux_64)
    PREFIX=-linux_64
    CFLAGS_GLOBAL += -mabi=64 -march=octeon -msoft-float -Dmain=appmain
    ASFLAGS_GLOBAL += -mabi=64 -march=octeon -msoft-float -Dmain=appmain
    LDFLAGS_GLOBAL += -mabi=64 -static -lrt -Wl,-T,$(OCTEON_ROOT)/target/lib/cvmx-shared-linux.ld
endif
ifeq (${OCTEON_TARGET},linux_n32)
    PREFIX=-linux_n32
    CFLAGS_GLOBAL += -mabi=n32 -march=octeon -msoft-float -Dmain=appmain
    ASFLAGS_GLOBAL += -mabi=n32 -march=octeon -msoft-float -Dmain=appmain
    LDFLAGS_GLOBAL += -mabi=n32 -static -lrt -Wl,-T,$(OCTEON_ROOT)/target/lib/cvmx-shared-linux-n32.ld
endif
ifeq (${OCTEON_TARGET},linux_uclibc)
    PREFIX=-linux_uclibc
    CFLAGS_GLOBAL += -muclibc -march=octeon -msoft-float -Dmain=appmain
    ASFLAGS_GLOBAL += -muclibc -march=octeon -msoft-float -Dmain=appmain
    LDFLAGS_GLOBAL += -muclibc -static -lrt -Wl,-T,$(OCTEON_ROOT)/target/lib/cvmx-shared-linux-n32.ld
endif
ifeq (${OCTEON_TARGET},linux_o32)
    PREFIX=-linux_o32
    CFLAGS_GLOBAL += -mabi=32 -march=octeon -msoft-float -Dmain=appmain
    ASFLAGS_GLOBAL += -mabi=32 -march=octeon -msoft-float -Dmain=appmain
    LDFLAGS_GLOBAL += -mabi=32 -static -lrt -Wl,-T,$(OCTEON_ROOT)/target/lib/cvmx-shared-linux-o32.ld
endif
ifeq (${OCTEON_TARGET},cvmx_n32)
    CFLAGS_GLOBAL += -DOCTEON_TARGET=cvmx_n32 -mabi=n32
    ASFLAGS_GLOBAL += -DOCTEON_TARGET=cvmx_n32 -mabi=n32
    LDFLAGS_GLOBAL += -mabi=n32
    PREFIX=-cvmx_n32
endif
ifeq (${OCTEON_TARGET},cvmx_64)
    CFLAGS_GLOBAL += -DOCTEON_TARGET=cvmx_64
    ASFLAGS_GLOBAL += -DOCTEON_TARGET=cvmx_64
endif

ifeq (linux,$(findstring linux,$(OCTEON_TARGET)))
    CC = mips64-octeon-linux-gnu-gcc
    AR = mips64-octeon-linux-gnu-ar
    LD = mips64-octeon-linux-gnu-ld
    STRIP = mips64-octeon-linux-gnu-strip
    OBJDUMP = mips64-octeon-linux-gnu-objdump
    NM = mips64-octeon-linux-gnu-nm
else
    CC = mipsisa64-octeon-elf-gcc
    AR = mipsisa64-octeon-elf-ar
    LD = mipsisa64-octeon-elf-ld
    STRIP = mipsisa64-octeon-elf-strip
    OBJDUMP = mipsisa64-octeon-elf-objdump
    NM = mipsisa64-octeon-elf-nm
endif

#  build object directory

OBJ_DIR = obj$(PREFIX)

#  standard compile line

COMPILE = $(CC) $(CFLAGS_GLOBAL) $(CFLAGS_LOCAL) -MD -c -o $@ $<

ASSEMBLE = $(CC) $(ASFLAGS_GLOBAL) $(ASFLAGS_LOCAL) -MD -c -o $@ $<

