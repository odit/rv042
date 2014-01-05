/*************************************************************************
Copyright (c) 2006 Cavium Networks (support@cavium.com). All rights
reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
3. Cavium Networks' name may not be used to endorse or promote products
derived from this software without specific prior written permission.

This Software, including technical data, may be subject to U.S. export
control laws, including the U.S. Export Administration Act and its
associated regulations, and may be subject to export or import
regulations in other countries. You warrant that You will comply
strictly in all respects with all such regulations and acknowledge that
you have the responsibility to obtain licenses to export, re-export or
import the Software.

TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
AND WITH ALL FAULTS AND CAVIUM MAKES NO PROMISES, REPRESENTATIONS OR
WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
RESPECT TO THE SOFTWARE, INCLUDING ITS CONDITION, ITS CONFORMITY TO ANY
REPRESENTATION OR DESCRIPTION, OR THE EXISTENCE OF ANY LATENT OR PATENT
DEFECTS, AND CAVIUM SPECIFICALLY DISCLAIMS ALL IMPLIED (IF ANY)
WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. THE ENTIRE
RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE LIES WITH YOU.

*************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include "cvmx-config.h"
#include "cvmx.h"
#include <octeon-app-init.h>
#include "cvmx-app-init.h"
#include "cvmx-sysinfo.h"







/**
 * This function checks to see if the software is compatible with the
 * chip it is running on.  This is called in the application startup code
 * and does not need to be called directly by the application.
 * Does not return if software is incompatible.
 * 
 * @param chip_id chip id that the software is being run on.
 * 
 * @return 0: runtime checking or exact version match
 *         1: chip is newer revision than compiled for, but software will run properly.
 */
int octeon_model_version_check(uint32_t chip_id)
{

#if OCTEON_IS_COMMON_BINARY()
    if (chip_id == OCTEON_CN38XX_PASS1)
    {
        printf("Runtime Octeon Model checking binaries do not support OCTEON_CN38XX_PASS1 chips\n");
#ifndef __linux__
        if (cvmx_sysinfo_get()->board_type == CVMX_BOARD_TYPE_SIM)
            CVMX_BREAK;
        while (1);
#else
        exit(-1);
#endif
    }
#else
    /* Check for special case of mismarked 3005 samples, and adjust cpuid */
    if (chip_id == OCTEON_CN3010_PASS1 && (cvmx_read_csr(0x80011800800007B8ull) & (1ull << 34)))
        chip_id |= 0x10;

    if ((OCTEON_MODEL & 0xffffff) != chip_id)
    {
        if (!OCTEON_IS_MODEL((OM_IGNORE_REVISION | chip_id)) || (OCTEON_MODEL & 0xffffff) > chip_id || (((OCTEON_MODEL & 0xffffff) ^ chip_id) & 0x10))
        {
            printf("ERROR: Software not configured for this chip\n"
                   "         Expecting ID=0x%08x, Chip is 0x%08x\n", (OCTEON_MODEL & 0xffffff), (unsigned int)chip_id);
            if ((OCTEON_MODEL & 0xffffff) > chip_id)
                printf("Refusing to run on older revision than program was compiled for.\n");
#ifndef __linux__
            if (cvmx_sysinfo_get()->board_type == CVMX_BOARD_TYPE_SIM)
                CVMX_BREAK;
            while (1);
#else
            exit(-1);
#endif
        }
        else
        {
            printf("\n###################################################\n");
            printf("WARNING: Software configured for older revision than running on.\n"
                   "         Compiled for ID=0x%08x, Chip is 0x%08x\n", (OCTEON_MODEL & 0xffffff), (unsigned int)chip_id);
            printf("###################################################\n\n");
            return(1);
        }
    }
#endif

    return(0);
}
