/***************************************************************************
 *  alg_dns.h : This file is part of 'ataga'
 *  created on: Mon Jun 20 14:21:08 CDT 2005
 *
 *  (c) 2005 by Lukasz Tomicki <lucas.tomicki@gmail.com>
 * 
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __ALG_DNS_H__
#define __ALG_DNS_H__

struct dns_query {
	u8* input_packet_start;
	u8* input_packet_end;
	u8* output_packet_start;
	u8* input_ptr;
	u8* output_ptr;
	u8 *dnptrs[2];
	u8 *plast_dnptr;
	u32 input_size;
	u16 direction; /* from */
	u32 buffer_remaining;
	NAT_Manager::addr_mapping *mapping;
};

bool v4_main(NAT_Manager::addr_mapping *mapping, char *buffer, u32 *size);
bool v6_main(NAT_Manager::addr_mapping *mapping, char *buffer, u32 *size);
const char *name_func(int code);
u16 port_func(int code);
bool translate_message(dns_query*);
bool analyse_header(dns_query*);
bool process_query_section(dns_query*);
bool process_resource_records(dns_query*, u32);
bool translate_reverse_name_v6(dns_query*, char*, int);
bool translate_reverse_name_v4(dns_query*, char*, int);
bool extract_v4_reverse_addr(char*, int);
bool extract_v6_reverse_addr(char*, int);
bool extract_v4_address(char*, in_addr*);
bool extract_v6_address(char*, in6_addr*);
bool create_v4_reverse_name(char*, in_addr *);
bool create_v6_reverse_name(char*, in6_addr *);
bool translate_rdata(dns_query*, u16, u16*);
int display_header_info(HEADER *pmessage_header);

#endif // __ALG_DNS_H__
