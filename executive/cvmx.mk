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

#
#  component Makefile fragment
#

#  standard component Makefile header
sp              :=  $(sp).x
dirstack_$(sp)  :=  $(d)
d               :=  $(dir)

#  component specification

LIBRARY := $(OBJ_DIR)/libcvmx.a

OBJS_$(d)  :=  \
	$(OBJ_DIR)/cvmx-zone.o \
	$(OBJ_DIR)/cvmx-fpa.o \
	$(OBJ_DIR)/cvmx-tim.o \
	$(OBJ_DIR)/cvmx-pko.o \
	$(OBJ_DIR)/cvmx-dfa.o \
	$(OBJ_DIR)/cvmx-sysinfo.o \
	$(OBJ_DIR)/cvmx-coremask.o \
	$(OBJ_DIR)/cvmx-bootmem.o \
	$(OBJ_DIR)/cvmx-bootmem-shared.o \
	$(OBJ_DIR)/cvmx-helper.o \
	$(OBJ_DIR)/cvmx-core.o \
	$(OBJ_DIR)/cvmx-log.o \
	$(OBJ_DIR)/cvmx-log-arc.o \
	$(OBJ_DIR)/cvmx-llm.o \
	$(OBJ_DIR)/cvmx-l2c.o \
	$(OBJ_DIR)/cvmx-l2c-asm.o \
	$(OBJ_DIR)/cvmx-flash.o \
	$(OBJ_DIR)/cvmx-cn3010-evb-hs5.o \
	$(OBJ_DIR)/cvmx-ebt3000.o \
	$(OBJ_DIR)/cvmx-spi.o \
	$(OBJ_DIR)/cvmx-spi4000.o \
	$(OBJ_DIR)/cvmx-twsi.o \
	$(OBJ_DIR)/cvmx-twsi.o \
	$(OBJ_DIR)/octeon-model.o \
	$(OBJ_DIR)/cvmx-thunder.o
ifeq (linux,$(findstring linux,$(OCTEON_TARGET)))
OBJS_$(d)  +=  \
	$(OBJ_DIR)/cvmx-app-init-linux.o
else
OBJS_$(d)  +=  \
	$(OBJ_DIR)/cvmx-interrupt.o \
	$(OBJ_DIR)/cvmx-interrupt-handler.o \
	$(OBJ_DIR)/cvmx-app-init.o \
	$(OBJ_DIR)/cvmx-malloc.o
endif

$(OBJS_$(d)):  CFLAGS_LOCAL := -I$(d) -O2 -g -W -Wall -Wno-unused-parameter

#  standard component Makefile rules

DEPS_$(d)   :=  $(OBJS_$(d):.o=.d)

LIBS_LIST   :=  $(LIBS_LIST) $(LIBRARY)

CLEAN_LIST  :=  $(CLEAN_LIST) $(OBJS_$(d)) $(DEPS_$(d)) $(LIBRARY)

-include $(DEPS_$(d))

$(LIBRARY): $(OBJS_$(d))
	$(AR) -cr $@ $^

$(OBJ_DIR)/%.o:	$(d)/%.c
	$(COMPILE)

$(OBJ_DIR)/%.o:	$(d)/%.S
	$(ASSEMBLE)

$(OBJ_DIR)/cvmx-app-init.o: $(d)/cvmx-app-init.c
	$(CC) $(CFLAGS_GLOBAL) $(CFLAGS_LOCAL) -O0 -MD -c -o $@ $<

$(OBJ_DIR)/cvmx-app-init-linux.o: $(d)/cvmx-app-init-linux.c
	$(CC) $(CFLAGS_GLOBAL) $(CFLAGS_LOCAL) -MD -c -Umain -o $@ $<

CFLAGS_SPECIAL := -I$(d) -I$(d)/cvmx-malloc -O2 -g -DUSE_CVM_THREADS=1 -D_REENTRANT

$(OBJ_DIR)/cvmx-malloc.o: $(d)/cvmx-malloc/malloc.c
	$(CC) $(CFLAGS_GLOBAL) $(CFLAGS_SPECIAL) -MD -c -o $@ $<

#  standard component Makefile footer

d   :=  $(dirstack_$(sp))
sp  :=  $(basename $(sp))
