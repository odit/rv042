/***********************license start***************
 * Copyright (c) 2003-2008, Cavium Networks. All rights reserved.
 * 
 * This software file (the "File") is owned and distributed by Cavium 
 * Networks ("Cavium") under the following dual licensing option: The dual 
 * licensing option gives you, the licensee, the choice between the following 
 * alternative licensing terms.  Once you have made an election to use the 
 * File under one of the following alternative licensing terms (license 
 * types) you are bound by the respective terms and you may distribute the 
 * file (or any derivative thereof), to the extent allowed by the respective 
 * licensing term, only if you (i) delete this introductory statement 
 * regarding the dual licensing option from the file you will distribute, 
 * (ii) delete the licensing term that you have elected NOT to use from the 
 * file you will distribute and (iii) follow the respective licensing term 
 * that you have elected to use with respect to the correct attribution or 
 * licensing term that you have to include with your distribution.  
 * 
 * ***
 * OCTEON SDK License Type 2:
 * 
 * IMPORTANT: Read this Agreement carefully before clicking on the "I accept" 
 * button to download the Software and/or before using the Software.  This 
 * License Agreement (the "Agreement") is a legal agreement between you, 
 * either an individual or a single legal entity ("You" or "you"), and Cavium 
 * Networks ("Cavium").  This Agreement governs your use of the Cavium 
 * software that can be downloaded after accepting this Agreement and/or that 
 * is accompanied by this Agreement (the "Software").  You must accept the 
 * terms of this Agreement before downloading and/or using the Software.  By 
 * clicking on the "I accept" button to download and/or by using the 
 * Software, you are indicating that you have read and understood, and assent 
 * to be bound by, the terms of this Agreement.  If you do not agree to the 
 * terms of the Agreement, you are not granted any rights whatsoever in the 
 * Software.  If you are not willing to be bound by these terms and 
 * conditions, you should not use or cease all use of the Software.  This 
 * Software is the property of Cavium Networks and constitutes the 
 * proprietary information of Cavium Networks.  You agree to take reasonable 
 * steps to prevent the disclosure, unauthorized use or unauthorized 
 * distribution of the Software to any third party.  
 * 
 * License Grant.  Subject to the terms and conditions of this Agreement, 
 * Cavium grants you a nonexclusive, non-transferable, worldwide, fully-paid 
 * and royalty-free license to 
 * 
 * (a) install, reproduce, and execute the executable version of the Software 
 * solely for your internal use and only (a) on hardware manufactured by 
 * Cavium, or (b) software of Cavium that simulates Cavium hardware; 
 * 
 * (b) create derivative works of any portions of the Software provided to 
 * you by Cavium in source code form, which portions enable features of the 
 * Cavium hardware products you or your licensees are entitled to use, 
 * provided that any such derivative works must be used only (a) on hardware 
 * manufactured by Cavium, or (b) software of Cavium that simulates Cavium 
 * hardware; and 
 * 
 * (c) distribute derivative works you created in accordance with clause (b) 
 * above, only in executable form and only if such distribution (i) 
 * reproduces the copyright notice that can be found at the very end of this 
 * Agreement and (ii) is pursuant to a binding license agreement that 
 * contains terms no less restrictive and no less protective of Cavium than 
 * this Agreement.  You will immediately notify Cavium if you become aware of 
 * any breach of any such license agreement.  
 * 
 * Restrictions.  The rights granted to you in this Agreement are subject to 
 * the following restrictions: Except as expressly set forth in this 
 * Agreement (a) you will not license, sell, rent, lease, transfer, assign, 
 * display, host, outsource, disclose or otherwise commercially exploit or 
 * make the Software, or any derivatives you create under this Agreement, 
 * available to any third party; (b) you will not modify or create derivative 
 * works of any part of the Software; (c) you will not access or use the 
 * Software in order to create similar or competitive products, components, 
 * or services; and (d), no part of the Software may be copied (except for 
 * the making of a single archival copy), reproduced, distributed, 
 * republished, downloaded, displayed, posted or transmitted in any form or 
 * by any means.  
 * 
 * Ownership.  You acknowledge and agree that, subject to the license grant 
 * contained in this Agreement and as between you and Cavium (a) Cavium owns 
 * all copies of and intellectual property rights to the Software, however 
 * made, and retains all rights in and to the Software, including all 
 * intellectual property rights therein, and (b) you own all the derivate 
 * works of the Software created by you under this Agreement, subject to 
 * Cavium's rights in the Software.  There are no implied licenses under this 
 * Agreement, and any rights not expressly granted to your hereunder are 
 * reserved by Cavium.  You will not, at any time, contest anywhere in the 
 * world Cavium's ownership of the intellectual property rights in and to the 
 * Software.  
 * 
 * Disclaimer of Warranties.  The Software is provided to you free of charge, 
 * and on an "As-Is" basis.  Cavium provides no technical support, warranties 
 * or remedies for the Software.  Cavium and its suppliers disclaim all 
 * express, implied or statutory warranties relating to the Software, 
 * including but not limited to, merchantability, fitness for a particular 
 * purpose, title, and non-infringement.  Cavium does not warrant that the 
 * Software and the use thereof will be error-free, that defects will be 
 * corrected, or that the Software is free of viruses or other harmful 
 * components.  If applicable law requires any warranties with respect to the 
 * Software, all such warranties are limited in duration to thirty (30) days 
 * from the date of download or first use, whichever comes first.  
 * 
 * Limitation of Liability.  Neither Cavium nor its suppliers shall be 
 * responsible or liable with respect to any subject matter of this Agreement 
 * or terms or conditions related thereto under any contract, negligence, 
 * strict liability or other theory (a) for loss or inaccuracy of data or 
 * cost of procurement of substitute goods, services or technology, or (b) 
 * for any indirect, incidental or consequential damages including, but not 
 * limited to loss of revenues and loss of profits.  Cavium's aggregate 
 * cumulative liability hereunder shall not exceed the greater of Fifty U.S.  
 * Dollars (U.S.$50.00) or the amount paid by you for the Software that 
 * caused the damage.  Certain states and/or jurisdictions do not allow the 
 * exclusion of implied warranties or limitation of liability for incidental 
 * or consequential damages, so the exclusions set forth above may not apply 
 * to you.  
 * 
 * Basis of Bargain.  The warranty disclaimer and limitation of liability set 
 * forth above are fundamental elements of the basis of the agreement between 
 * Cavium and you.  Cavium would not provide the Software without such 
 * limitations.  The warranty disclaimer and limitation of liability inure to 
 * the benefit of Cavium and Cavium's suppliers.  
 * 
 * Term and Termination.  This Agreement and the licenses granted hereunder 
 * are effective on the date you accept the terms of this Agreement, download 
 * the Software, or use the Software, whichever comes first, and shall 
 * continue unless this Agreement is terminated pursuant to this section.  
 * This Agreement immediately terminates in the event that you materially 
 * breach any of the terms hereof.  You may terminate this Agreement at any 
 * time, with or without cause, by destroying any copies of the Software in 
 * your possession.  Upon termination, the license granted hereunder shall 
 * terminate but the Sections titled "Restrictions", "Ownership", "Disclaimer 
 * of Warranties", "Limitation of Liability", "Basis of Bargain", "Term and 
 * Termination", "Export", and "Miscellaneous" will remain in effect.  
 * 
 * Export.  The Software and related technology are subject to U.S.  export 
 * control laws and may be subject to export or import regulations in other 
 * countries.  You agree to strictly comply with all such laws and 
 * regulations and acknowledges that you have the responsibility to obtain 
 * authorization to export, re-export, or import the Software and related 
 * technology, as may be required.  You will indemnify and hold Cavium 
 * harmless from any and all claims, losses, liabilities, damages, fines, 
 * penalties, costs and expenses (including attorney's fees) arising from or 
 * relating to any breach by you of your obligations under this section.  
 * Your obligations under this section shall survive the expiration or 
 * termination of this Agreement.  
 * 
 * Miscellaneous.  Neither the rights nor the obligations arising under this 
 * Agreement are assignable by you, and any such attempted assignment or 
 * transfer shall be void and without effect.  This Agreement shall be 
 * governed by and construed in accordance with the laws of the State of 
 * California without regard to any conflicts of laws provisions that would 
 * require application of the laws of another jurisdiction.  Any action under 
 * or relating to this Agreement shall be brought in the state and federal 
 * courts located in California, with venue in the courts located in Santa 
 * Clara County and each party hereby submits to the personal jurisdiction of 
 * such courts; provided, however, that nothing herein will operate to 
 * prohibit or restrict Cavium from filing for and obtaining injunctive 
 * relief from any court of competent jurisdiction.  The United Nations 
 * Convention on Contracts for the International Sale of Goods shall not 
 * apply to this Agreement.  In the event that any provision of this 
 * Agreement is found to be contrary to law, then such provision shall be 
 * construed as nearly as possible to reflect the intention of the parties, 
 * with the other provisions remaining in full force and effect.  Any notice 
 * to you may be provided by email.  This Agreement constitutes the entire 
 * agreement between the parties and supersedes all prior or contemporaneous, 
 * agreements, understandings and communications between the parties, whether 
 * written or oral, pertaining to the subject matter hereof.  Any 
 * modifications of this Agreement must be in writing and agreed to by both 
 * parties.  
 * 
 * Copyright (c) 2003-2008, Cavium Networks. All rights reserved.
 * 
 * ***
 * 
 * OCTEON SDK License Type 4:
 * 
 * Author: Cavium Networks 
 * 
 * Contact: support@caviumnetworks.com 
 * This file is part of the OCTEON SDK
 * 
 * Copyright (c) 2007 Cavium Networks 
 * 
 * This file is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License, Version 2, as published by 
 * the Free Software Foundation. 
 * 
 * This file is distributed in the hope that it will be useful, 
 * but AS-IS and WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, TITLE, or NONINFRINGEMENT. 
 * See the GNU General Public License for more details. 
 * 
 * You should have received a copy of the GNU General Public License 
 * along with this file; if not, write to the Free Software 
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 * or visit http://www.gnu.org/licenses/. 
 * 
 * This file may also be available under a different license from Cavium. 
 * Contact Cavium Networks for more information
 ***********************license end**************************************/

/* Structures used to pass information from the bootloader to the application.
   This should not be used by the application directly.  */

#ifndef __OCTEON_APP_INIT_H__
#define __OCTEON_APP_INIT_H__

/* Define to allow conditional compilation when new items are added */
/* Version 2: added OCTEON_BL_FLAG_CONSOLE_PCI */
/* Version 3: added OCTEON_BL_FLAG_BREAK */
#define OCTEON_APP_INIT_H_VERSION   3

/* Macro indicates that bootmem related structures are now in
   cvmx-bootmem.h.  */
#define OCTEON_APP_INIT_BOOTMEM_STRUCTS_MOVED

typedef enum
{
  /* If set, core should do app-wide init, only one core per app will have 
     this flag set.  */ 
  BOOT_FLAG_INIT_CORE     = 1,  
  OCTEON_BL_FLAG_DEBUG    = 1 << 1,
  OCTEON_BL_FLAG_NO_MAGIC = 1 << 2,
  OCTEON_BL_FLAG_CONSOLE_UART1 = 1 << 3,  /* If set, use uart1 for console */
  OCTEON_BL_FLAG_CONSOLE_PCI = 1 << 4,  /* If set, use PCI console */
  OCTEON_BL_FLAG_BREAK	  = 1 << 5, /* Call exit on break on serial port */
  /* Be sure to update OCTEON_APP_INIT_H_VERSION when new fields are added
  ** and to conditionalize the new flag's usage based on the version. */
} octeon_boot_descriptor_flag_t;

#define OCTEON_CURRENT_DESC_VERSION     6
#define OCTEON_ARGV_MAX_ARGS            (64)

#define OCTOEN_SERIAL_LEN 20

/* Bootloader structure used to pass info to Octeon executive startup code.
   NOTE: all fields are deprecated except for:
   * desc_version
   * desc_size,
   * heap_base
   * heap_end
   * exception_base_addr
   * flags
   * argc
   * argv
   * desc_size
   * cvmx_desc_vaddr
   * debugger_flags_base_addr

   All other fields have been moved to the cvmx_descriptor, and the new 
   fields should be added there. They are left as placeholders in this 
   structure for binary compatibility.  */
typedef struct
{   
  /* Start of block referenced by assembly code - do not change! */
  uint32_t desc_version;
  uint32_t desc_size;
  uint64_t stack_top;
  uint64_t heap_base;
  uint64_t heap_end;
  /* Only used by bootloader */
  uint64_t entry_point;   
  uint64_t desc_vaddr;
  /* End of This block referenced by assembly code - do not change! */
  uint32_t exception_base_addr;
  uint32_t stack_size;
  uint32_t heap_size;
  /* Argc count for application. */
  uint32_t argc;  
  uint32_t argv[OCTEON_ARGV_MAX_ARGS];
  uint32_t flags;
  uint32_t core_mask;
  /* DRAM size in megabyes. */
  uint32_t dram_size;  
  /* physical address of free memory descriptor block. */
  uint32_t phy_mem_desc_addr;  
  /* used to pass flags from app to debugger. */
  uint32_t debugger_flags_base_addr;  
  /* CPU clock speed, in hz. */
  uint32_t eclock_hz;  
  /* DRAM clock speed, in hz. */
  uint32_t dclock_hz;  
  /* SPI4 clock in hz. */
  uint32_t spi_clock_hz;  
  uint16_t board_type;
  uint8_t board_rev_major;
  uint8_t board_rev_minor;
  uint16_t chip_type;
  uint8_t chip_rev_major;
  uint8_t chip_rev_minor;
  char board_serial_number[OCTOEN_SERIAL_LEN];
  uint8_t mac_addr_base[6];
  uint8_t mac_addr_count;
  uint64_t cvmx_desc_vaddr;
} octeon_boot_descriptor_t;

/* Debug flags bit definitions.  */
#define DEBUG_FLAG_CORE_DONE    0x1

#endif /* __OCTEON_APP_INIT_H__ */
