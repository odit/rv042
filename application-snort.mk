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
#  application Makefile fragment
#
#  $Id: application-snort.mk,v 1.1 2009-10-15 07:03:05 tt Exp $ $Name: not supported by cvs2svn $
#

#  target to create object directory

$(OBJ_DIR):
	mkdir $(OBJ_DIR)

#  applications object suffix rule

$(OBJ_DIR)/%.o: %.c
	$(COMPILE)

$(OBJ_DIR)/%.o: %.S
	$(ASSEMBLE)

#  application config check and rules

CVMX_CONFIG = config/cvmx-config.h
CVMX_OTHER_CONFIGS := config/*-config.h
CVMX_OTHER_CONFIGS := $(shell echo $(CVMX_OTHER_CONFIGS) | sed 's/config\/cvmx-config.h//')

$(CVMX_CONFIG): $(CVMX_OTHER_CONFIGS)
	cvmx-config $(CVMX_OTHER_CONFIGS)

#  application object dependencies

-include $(OBJS:.o=.d)

#  special rule to re-compile if make command line variables TARGET_SIM or
#  TARGET_LINUX change
MATCH=${findstring DUSE_RUNTIME_MODEL_CHECKS=1, ${OCTEON_CPPFLAGS_GLOBAL_ADD}}
ifeq (${MATCH}, DUSE_RUNTIME_MODEL_CHECKS=1)
# We are using runtime model detection, so don't put model in string
MADE_WITH = $(OBJ_DIR)/made_with-OCTEON_MODEL={runtime}-OCTEON_CPPFLAGS_GLOBAL_ADD={$(OCTEON_CPPFLAGS_GLOBAL_ADD)}.
else
MADE_WITH = $(OBJ_DIR)/made_with-OCTEON_MODEL={$(OCTEON_MODEL)}-OCTEON_CPPFLAGS_GLOBAL_ADD={$(OCTEON_CPPFLAGS_GLOBAL_ADD)}.
endif

MADE_WITH_ALL = $(OBJ_DIR)/made_with-*

$(MADE_WITH):
	mkdir -p $(OBJ_DIR)
	rm -f $(MADE_WITH_ALL)
	touch $(MADE_WITH)

$(TARGET) $(OBJS) $(CLEAN_LIST): $(MADE_WITH)

#  application build target

# The user decides if the build will create the cdb files for use with
# the EDB debugger or the stripped binaries used to save space and
# reduce download time by defining the EXTRA_CVMX_APPLICATION_TARGETS
# environment variable.
#
# $ setenv EXTRA_CVMX_APPLICATION_TARGETS '$(TARGET).stp $(TARGET).cdb'
#
# If the variable is not defined the build works in the usual way.

#   The user decides if the build will create archives by defining the
#   CVMX_ARCHIVE_DIRECTORY environment variable.
#   
#   $ setenv CVMX_ARCHIVE_DIRECTORY $HOME/archive
#   
#   If the variable is not defined the build works in the usual way.
#   Building will create a unique subdirectory for each OCTEON_MODEL
#   configuration.

ARCHIVE_FILES = $(TARGET) $(EXTRA_CVMX_APPLICATION_TARGETS)

archive: $(ARCHIVE_FILES)
	-@ if [ "${CVMX_ARCHIVE_DIRECTORY}" != "" ] ; then \
		mkdir -p ${CVMX_ARCHIVE_DIRECTORY}/${OCTEON_MODEL} ; \
		cp -f $(ARCHIVE_FILES) ${CVMX_ARCHIVE_DIRECTORY}/${OCTEON_MODEL}/ ; \
		chmod -R a+r ${CVMX_ARCHIVE_DIRECTORY}/${OCTEON_MODEL} ; \
	 fi ; \

application-target: $(TARGET) $(EXTRA_CVMX_APPLICATION_TARGETS) archive


ifdef TARGET_SIM
$(TARGET):
	echo "TARGET_SIM is deprecated use OCTEON_TARGET=cvmx_64/cvmx_n32/linux_64/linux_n32"
	exit 0
else
$(TARGET): $(CVMX_CONFIG) $(OBJ_DIR) $(OBJS) $(LIBS_LIST)
	$(CC) $(OBJS) $(LDFLAGS_PATH) -L/tmp/root-rootfs/usr/lib -L/usr/local/cavium/tools-gcc-4.1/mips64-octeon-linux-gnu/sys-root/usr/lib64/ $(LIBS_LIST) $(snort_LDADD) $(LIBS) -lc -lnss_files -lnss_dns -lresolv $(LDFLAGS_GLOBAL) -o $@
endif


$(TARGET).stp: $(TARGET)
	$(STRIP) -o $(TARGET).stp $(TARGET)


$(TARGET).cdb: $(TARGET)
#	Use cdbtrans to generate $(TARGET).cdb for use by the EDB source
#	debugger.  Hint: The user should set the environment variable
#	CDBTRANS_OPTIONS to pass personalized options into cdbtrans.  If
#	there exists a Windows path (through Samba, etc) to the source files
#	where they were built on Linux the user may use the -x option to
#	transform the source file paths from unix paths to equivalent
#	Windows paths.  This allows the EJTAG EDB debugger running on
#	Windows to display the source files from where they were built on
#	Linux.  Also note that multiple uses of the -x option are allowed.
#	Example:
#	$ setenv CDBTRANS_OPTIONS  "-x /nfs/user/linux x:\linux"

#	Insure that cdbtrans exists in the user's path before running it.
ifeq (linux,$(findstring linux,$(OCTEON_TARGET)))
	!(which cdbtrans64 > /dev/null 2>&1) || (cdbtrans64 -o $(TARGET) $(TARGET) ${CDBTRANS_LINUX_OPTIONS})
else
	!(which cdbtrans > /dev/null 2>&1) || (cdbtrans -o $(TARGET) $(TARGET) ${CDBTRANS_OPTIONS})
endif
