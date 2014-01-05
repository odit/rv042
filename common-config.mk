# This makefile fragment contains the architecture independent
# configuration options needed for proper simple exectutive compilation.
# This should be included by any makefiles using all or portions of the
# simple executive, and the CFLAGS_COMMON_CONFIG variable should be added
# to the CFLAGS used to compile the simple executive with.



DEPRECATED_MODELS= OCTEON_PASS2 OCTEON_PASS1
MODELS_STRING=  $(shell cat ${OCTEON_ROOT}/octeon-models.txt)

ifndef OCTEON_MODEL
  ${error OCTEON_MODEL not set.  Supported values: ${MODELS_STRING}}
endif

# Do 36XX to 38XX translation
TMP := $(subst CN36XX,CN38XX,${OCTEON_MODEL})
OCTEON_MODEL:=${TMP}

ifndef OCTEON_ROOT
  ${error OCTEON_ROOT not set}
endif

#
MATCHED_MODEL=${findstring ${OCTEON_MODEL}, ${DEPRECATED_MODELS}}
ifeq (${MATCHED_MODEL}, ${OCTEON_MODEL})
${warning OCTEON_MODEL of ${OCTEON_MODEL} is deprecated. Please use a model from ${OCTEON_ROOT}/octeon-models.txt.}
else
MATCHED_MODEL=${findstring ${OCTEON_MODEL}, ${MODELS_STRING}}

ifneq (${MATCHED_MODEL}, ${OCTEON_MODEL})
${error Invalid OCTEON_MODEL: ${OCTEON_MODEL}.  Valid values: ${MODELS_STRING}}
endif



endif




CFLAGS_COMMON_CONFIG += -DOCTEON_MODEL=${MATCHED_MODEL}
