/***************************************************************************
 *  alg_dns.cc : This file is part of 'ataga'
 *  created on: Thu Jun 16 16:44:20 CDT 2005
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

#include <errno.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include "ataga.h"
#include "alg.h"
#include "alg_dns.h"

in6_addr v6_prefix;

bool v4_main(NAT_Manager::addr_mapping *mapping, char *buffer, u32 *size)
{
	u8 out_packet[2 * (packet_size - sizeof(iphdr) - sizeof(udphdr))];
	iphdr *ip = (iphdr*)(buffer);
	if (ip->protocol != IPPROTO_UDP)
		return false;
	
	dns_query dns;
	dns.input_packet_start = (u8*)buffer + ip->ihl * 4 + sizeof(udphdr);
	dns.input_packet_end = (u8*)buffer + htons(ip->tot_len);
	dns.output_packet_start = out_packet;
	dns.dnptrs[0] = out_packet;
	dns.dnptrs[1] = 0;
	dns.plast_dnptr  = *dns.dnptrs + sizeof(dns.dnptrs) / sizeof(dns.dnptrs[0]);
	dns.input_ptr = dns.input_packet_start;
	dns.output_ptr = out_packet;
	dns.input_size = htons(ip->tot_len) - sizeof(udphdr) - sizeof(iphdr);
	dns.direction = IPPROTO_IPV6;
	dns.buffer_remaining = (packet_size - sizeof(iphdr) - sizeof(udphdr));
	dns.mapping = mapping;
	
	if (!translate_message(&dns))
		return false;
	
	memcpy(buffer + ip->ihl * 4 + sizeof(udphdr), out_packet, dns.output_ptr - dns.output_packet_start);
	ip->tot_len = htons(dns.output_ptr - dns.output_packet_start + ip->ihl * 4 + sizeof(udphdr));
	udphdr *udp = (udphdr*)(buffer + ip->ihl * 4);
	udp->len = htons(dns.output_ptr - dns.output_packet_start + sizeof(udphdr));
	
#ifdef _DEBUG_
	printf("payload size updated original(%u), new(%u)\n", dns.input_size, dns.output_ptr - dns.output_packet_start);
#endif // _DEBUG_

	*size = dns.output_ptr - dns.output_packet_start + ip->ihl * 4 + sizeof(udphdr);
	
	return true;
}

bool v6_main(NAT_Manager::addr_mapping *mapping, char *buffer, u32 *size)
{
	u8 out_packet[2 * (packet_size - sizeof(ip6_hdr) - sizeof(udphdr))];
	ip6_hdr *ip6 = (ip6_hdr*)(buffer);
	if (ip6->ip6_nxt != IPPROTO_UDP)
		return false;
	
	dns_query dns;
	dns.input_packet_start = (u8*)buffer + sizeof(ip6_hdr) + sizeof(udphdr);
	dns.input_packet_end = (u8*)buffer + htons(ip6->ip6_plen) + sizeof(ip6_hdr);
	dns.output_packet_start = out_packet;
	dns.dnptrs[0] = out_packet;
	dns.dnptrs[1] = 0;
	dns.plast_dnptr  = *dns.dnptrs + sizeof(dns.dnptrs) / sizeof(dns.dnptrs[0]);
	dns.input_ptr = dns.input_packet_start;
	dns.output_ptr = out_packet;
	dns.input_size = htons(ip6->ip6_plen) - sizeof(udphdr);
	dns.direction = IPPROTO_IP;
	dns.buffer_remaining = (packet_size - sizeof(ip6_hdr) - sizeof(udphdr));
	dns.mapping = mapping;
	
	if (!translate_message(&dns))
		return false;
	
	memcpy(buffer + sizeof(ip6_hdr) + sizeof(udphdr), out_packet, dns.output_ptr - dns.output_packet_start);
	ip6->ip6_plen = htons(dns.output_ptr - dns.output_packet_start + sizeof(udphdr));
	udphdr *udp = (udphdr*)(buffer + sizeof(ip6_hdr));
	udp->len = htons(dns.output_ptr - dns.output_packet_start + sizeof(udphdr));
	
#ifdef _DEBUG_
	printf("payload size updated original(%u), new(%u)\n", dns.input_size, dns.output_ptr - dns.output_packet_start);
#endif // _DEBUG_

	*size = dns.output_ptr - dns.output_packet_start + sizeof(ip6_hdr) + sizeof(udphdr);
	
	return true;	
}

const char *name_func(int code)
{
	static char name[] = "Domain Name Service";
	
	if (code)
		memcpy(&v6_prefix, (void*)code, sizeof(in6_addr));
	
	return name;
}

u16 port_func(int code)
{
	static u16 port(53);
	
	return port;
}

bool init_plugin(bool (*p)(NAT_Manager::addr_mapping*, int*, int*, u16, seq_action))
{
	return true;
}

bool translate_message(dns_query *dns)
{
#ifdef _DEBUG_
	printf("translate_message() called\n");
#endif // _DEBUG_
	
	HEADER *input_header = (HEADER*)dns->input_packet_start;
	
	analyse_header(dns);
	
	if (!process_query_section(dns))
		return false;
	
	if (!process_resource_records(dns, ntohs(input_header->ancount)))
		return false;
	
	if (!process_resource_records(dns, ntohs(input_header->nscount)))
		return false;
	
	if (!process_resource_records(dns, ntohs(input_header->arcount)))
		return false;
	
#ifdef _DEBUG_
	printf("translate_message() returning true\n");
#endif // _DEBUG_
	
	return true;
}

bool analyse_header(dns_query *dns)
{
	memcpy(dns->output_ptr, dns->input_ptr, HFIXEDSZ);

	dns->input_ptr += HFIXEDSZ;
	dns->output_ptr += HFIXEDSZ;
	
	return true;
}

bool process_query_section(dns_query *dns)
{
#ifdef _DEBUG_
	printf("process_query_section() called\n");
	printf("direction coming from: %s\n", dns->direction == IPPROTO_IP ? "v4" : "v6");
#endif // _DEBUG_
	
	HEADER *hdr = (HEADER*)dns->input_packet_start;
	u32 q_num(htons(hdr->qdcount));
	
	for (u32 query_no(0); query_no < q_num; ++query_no) {
		char query_name[MAXDNAME];
		int	qname_length = dn_expand(dns->input_packet_start, dns->input_packet_end, dns->input_ptr, query_name, MAXDNAME);
		
		if (qname_length < 0) {
			return false;
		}

#ifdef _DEBUG_
	printf("qname_length(%u)\n", qname_length);
#endif // _DEBUG_
		
		dns->input_ptr += qname_length;
		
		u16 query_type, query_class;
		GETSHORT(query_type, dns->input_ptr);
		GETSHORT(query_class, dns->input_ptr);

#ifdef _DEBUG_
	printf("query_type(%u) query_class(%u)\n", query_type, query_class);
#endif // _DEBUG_
		
		if (query_class == C_IN) {
			if (dns->direction == IPPROTO_IP) {
				switch (query_type) {
					case T_A:
						if (dns->mapping->v6_seq_offset != T_A)
							query_type = T_AAAA;
					break;
					
					case T_AAAA:
						query_type = T_A;
					break;
					
					case T_PTR:
						if (!translate_reverse_name_v4(dns, query_name, MAXDNAME))
							return false;
						
						else 
							qname_length = strlen(query_name);
					break;
				}
			} else if (dns->direction == IPPROTO_IPV6) {
				switch (query_type) {
					case T_A:
						dns->mapping->v6_seq_offset = T_A;
					break;
					
					case T_AAAA:
						dns->mapping->v6_seq_offset = T_AAAA;
						query_type = T_A;
					break;
				
					case T_PTR:
						if (!translate_reverse_name_v6(dns, query_name, MAXDNAME))
							return false;
						
						else
							qname_length = strlen(query_name);
					break;
				}
			}
		}
		
#ifdef _DEBUG_
	printf("calling dn_comp\n");
#endif // _DEBUG_
		
		qname_length = dn_comp(query_name, dns->output_ptr, packet_size, dns->dnptrs, &dns->plast_dnptr);
		if (qname_length < 0) {
			
#ifdef _DEBUG_
	printf("dn_comp: %s\n", strerror(errno));
#endif // _DEBUG_
			return false;
		}
#ifdef _DEBUG_
	printf("qname_length(%u)\n", qname_length);
#endif // _DEBUG_
		
		if (dns->buffer_remaining < qname_length)
			return false;
		
		dns->output_ptr += qname_length;
		dns->buffer_remaining -= qname_length;
		
#ifdef _DEBUG_
	printf("query_type(%u) query_class(%u)\n", query_type, query_class);
#endif // _DEBUG_
		
		if (dns->buffer_remaining < sizeof(u16) * 2)
			return false;
		
		PUTSHORT(query_type, dns->output_ptr);
		PUTSHORT(query_class, dns->output_ptr);
		dns->buffer_remaining -= sizeof(u16) * 2;
	}
	
	return true;
}

bool process_resource_records(dns_query *dns, u32 rr_count)
{
#ifdef _DEBUG_
	printf("process_resource_records() called rr_count(%u)\n", rr_count);
#endif // _DEBUG_
	
	for (u32 rr_no(0); rr_no < rr_count; ++rr_no) {
		char rr_name[MAXDNAME];
		
#ifdef _DEBUG_
	printf("calling dn_expand\n");
#endif // _DEBUG_
		int	name_length = dn_expand(dns->input_packet_start, dns->input_packet_end, dns->input_ptr, rr_name, MAXDNAME);
		
		if (name_length < 0) {
			
#ifdef _DEBUG_
	printf("dn_expand: %s\n", strerror(errno));
#endif // _DEBUG_
			return false;
		}
		
#ifdef _DEBUG_
	printf("name_length(%u)\n", name_length);
#endif // _DEBUG_
		
		dns->input_ptr += name_length;

		u16	rr_typ, rr_class, out_rr_typ;
		u32	rr_ttl;

		GETSHORT(rr_typ, dns->input_ptr);
		GETSHORT(rr_class, dns->input_ptr);
		GETLONG(rr_ttl, dns->input_ptr);
#ifdef _DEBUG_
	printf("rr_typ(%u) rr_class(%u)\n", rr_typ, rr_class);
#endif // _DEBUG_
		out_rr_typ = rr_typ;
		
		if (rr_class == C_IN) {
			if (dns->direction == IPPROTO_IP) {
				switch (rr_typ) {
					case T_A:
						if (dns->mapping->v6_seq_offset != T_A)
							out_rr_typ = T_AAAA;
					break;
					
					case T_AAAA:
						out_rr_typ = T_A;
					break;
					
					case T_PTR:
							if (!translate_reverse_name_v4(dns, rr_name, MAXDNAME))
								return false;
						
						else 
							name_length = strlen(rr_name);
					break;
				}
			} else if (dns->direction == IPPROTO_IPV6) {
				switch (rr_typ) {
					case T_AAAA:
						out_rr_typ = T_A;
					break;
				
					case T_PTR:
						if (!translate_reverse_name_v6(dns, rr_name, MAXDNAME))
							return false;
						
						else
							name_length = strlen(rr_name);
					break;
				}
			}
		}
		
		name_length = dn_comp(rr_name, dns->output_ptr, packet_size, dns->dnptrs, &dns->plast_dnptr);
		if (name_length < 0)
			return false;

		if (dns->buffer_remaining < name_length)
			return false;
		
		dns->output_ptr += name_length;
		dns->buffer_remaining -= name_length;
	
		if (dns->buffer_remaining < sizeof(u16) * 3 + sizeof(u32))
			return false;
		
		PUTSHORT(out_rr_typ, dns->output_ptr);
		PUTSHORT(rr_class, dns->output_ptr);
		PUTLONG(rr_ttl, dns->output_ptr);

		u16 rr_rdlength;
		GETSHORT(rr_rdlength, dns->input_ptr);
		
		u16 *p_rr_rdlength = (u16*)dns->output_ptr;
		PUTSHORT(rr_rdlength, dns->output_ptr);
		dns->buffer_remaining -= sizeof(u16) * 3 + sizeof(u32);

		u16 out_rr_rdlength(rr_rdlength);
		
		if (!translate_rdata(dns, out_rr_typ, &out_rr_rdlength))
			return false;

		if (out_rr_rdlength != rr_rdlength)
			PUTSHORT(out_rr_rdlength, p_rr_rdlength);
	}
	
#ifdef _DEBUG_
	printf("process_resource_records() returning true\n");
#endif // _DEBUG_
	return true;
}

bool translate_rdata(dns_query *dns, u16 rr_typ, u16 *rr_rdlength)
{
#ifdef _DEBUG_
	printf("translate_rdata() called\n");
#endif // _DEBUG_
	char dname[MAXDNAME];
	u16 preference;
	int dn_length(0);
	u32 serial, refresh, retry, expire, minimum;
	u8 *start_pos = dns->output_ptr;
	
	switch (rr_typ) {
		case T_AAAA:
#ifdef _DEBUG_
	printf("rr_typ == T_A\n");
#endif // _DEBUG_
			if (dns->buffer_remaining < 16)
				return false;
			
			memcpy(dns->output_ptr, &v6_prefix, 12);
			dns->output_ptr += 12;
		
			memcpy(dns->output_ptr, dns->input_ptr, sizeof(in_addr));
			dns->input_ptr += sizeof(in_addr);
			dns->output_ptr += sizeof(in_addr);
			dns->buffer_remaining -= sizeof(in_addr);

			break;
		
		case T_SOA:
			dn_length = dn_expand(dns->input_packet_start, dns->input_packet_end, dns->input_ptr, dname, MAXDNAME);
            if (dn_length < 0)
            	return false;

			dns->input_ptr += dn_length;
				
#ifdef _DEBUG_
			printf("dns_alg: mname=%s, len=%d, ", dname, dn_length);
#endif // _DEBUG_
			
			dn_length = dn_comp(dname, dns->output_ptr, MAXCDNAME, dns->dnptrs, &dns->plast_dnptr);
			if (dn_length < 0)
				return false;
	
			if (dns->buffer_remaining < dn_length)
				return false;
			
			dns->buffer_remaining -= dn_length;
			dns->output_ptr += dn_length;
	
			dn_length = dn_expand(dns->input_packet_start, dns->input_packet_end, dns->input_ptr, dname, MAXDNAME);
			if (dn_length < 0)
				return false;
			
#ifdef _DEBUG_
			printf("dns_alg: rname=%s, len=%d, ", dname, dn_length);
#endif // _DEBUG_
		
			dns->input_ptr += dn_length;
	
			dn_length = dn_comp(dname, dns->output_ptr, MAXCDNAME, dns->dnptrs, &dns->plast_dnptr);
			if (dn_length < 0)
				return false;
	
			if (dns->buffer_remaining < dn_length)
				return false;
			
			dns->buffer_remaining -= dn_length;
			dns->output_ptr += dn_length;

			if (dns->buffer_remaining < sizeof(long) * 5)
				return false;
			
			dns->buffer_remaining -= dn_length;
			GETLONG(serial, dns->input_ptr);
			GETLONG(refresh, dns->input_ptr);
			GETLONG(retry, dns->input_ptr);
			GETLONG(expire, dns->input_ptr);
			GETLONG(minimum, dns->input_ptr);

#ifdef _DEBUG_
			printf("dns_alg: %d %d %d %d %d\n", serial, refresh, retry, expire, minimum);
#endif // _DEBUG_

			PUTLONG(serial, dns->output_ptr);
			PUTLONG(refresh, dns->output_ptr);
			PUTLONG(retry, dns->output_ptr);
			PUTLONG(expire, dns->output_ptr);
			PUTLONG(minimum, dns->output_ptr);
			
			dns->buffer_remaining -= sizeof(long) * 5;
			
			break;
			
		case T_MX:
			if (dns->buffer_remaining < sizeof(u16))
				return false;
			
			GETSHORT(preference, dns->input_ptr);
			PUTSHORT(preference, dns->output_ptr);
			dns->buffer_remaining -= sizeof(u16);
 
			dn_length = dn_expand(dns->input_packet_start, dns->input_packet_end, dns->input_ptr, dname, MAXDNAME);
            
			if (dn_length < 0)
            	return false;
			
			dns->input_ptr += dn_length;

			dn_length = dn_comp(dname, dns->output_ptr, MAXCDNAME, dns->dnptrs, &dns->plast_dnptr);
			
			if (dn_length < 0)
				return false;
			
			if (dns->buffer_remaining < dn_length)
				return false;
			
			dns->output_ptr += dn_length;
			dns->buffer_remaining -= dn_length;

			break;
			
		case T_CNAME:
		case T_NS:
		case T_PTR:
		
			dn_length = dn_expand(dns->input_packet_start, dns->input_packet_end, dns->input_ptr, dname, MAXDNAME);
			
			if (dn_length < 0)
				return false;
	
			dns->input_ptr += dn_length;

			dn_length = dn_comp(dname, dns->output_ptr, MAXCDNAME, dns->dnptrs, &dns->plast_dnptr);
			
			if (dn_length < 0)
				return false;
			
			if (dns->buffer_remaining < dn_length)
				return false;
			
			dns->output_ptr += dn_length;
			dns->buffer_remaining -= dn_length;
	
			break;
			
		default:
			if (dns->buffer_remaining < *rr_rdlength)
				return false;
			
			memcpy(dns->output_ptr, dns->input_ptr, *rr_rdlength);
			dns->output_ptr += *rr_rdlength;
			dns->input_ptr += *rr_rdlength;
			dns->buffer_remaining -= *rr_rdlength;

			break;
	}

	*rr_rdlength = dns->output_ptr - start_pos;
	
	return true;
}

bool translate_reverse_name_v4(dns_query *dns, char *name, int len)
{		
	in_addr v4_address;
	in6_addr v6_address;

	if (!extract_v4_address(name, &v4_address))
		return true;
	
	if (dns->mapping->v6_seq_offset != T_AAAA) {
		dns->mapping->v6_seq_offset = T_A;
		return true;
	}
		
	memcpy(&v6_address, &v6_prefix, 12);
	memcpy((u8*)&v6_address + 12, &v4_address, sizeof(in_addr));
		
	if (!create_v6_reverse_name(name, &v6_address))
		return false;
	
	dns->mapping->v6_seq_offset = T_A;
	
	return true;
}

bool translate_reverse_name_v6(dns_query *dns, char *name, int len)
{
	in_addr v4_address;
	in6_addr v6_address;

	if (!extract_v6_address(name, &v6_address))
		return true;

	if (dns->mapping->v6_seq_offset != T_A) {
		dns->mapping->v6_seq_offset = T_AAAA;
		return true;
	}
	
	if (memcmp(&v6_address, &v6_prefix, 12))
		return false;
	
	memcpy(&v4_address, (u8*)&v6_address + 12, sizeof(in_addr));
	
	if (!create_v4_reverse_name(name, &v4_address))
		return false;
	
	dns->mapping->v6_seq_offset = T_AAAA;
	
	return true;
}

bool extract_v4_address(char *reverse_name, in_addr *dst)
{
	int rn_len = strlen(reverse_name);

	if ((rn_len < 14) || (strncasecmp(reverse_name + (rn_len - 13), ".in-addr.arpa", 13)))
		return false;
	
	char *labels[6];
	labels[0] = reverse_name;
	
	int num_labels(1);
	char *pc = reverse_name + 1;
	
	while (*pc != 0) {
		if (*pc == '.') {
			*pc++ = 0;
			labels[num_labels++] = pc;
		} else 
			pc++;
	}

	if (num_labels != 6) 
		return 0;

	char address[16];
	address[0] = 0;
	
	for (u32 i(num_labels - 2); i > 0; --i) {
		strcat(address, labels[i - 1]);
		if (i > 1)
			strcat(address, ".");
	}

	if (inet_pton(AF_INET, address, (void *)dst) == 1)
		return true;
	
	return false;
}

bool extract_v6_address(char *reverse_name, in6_addr *dst)
{
	if (strlen(reverse_name)!=71 || strcasecmp(reverse_name+64, "IP6.INT"))
		return false;

	char address[40];

	for (u32 i(0); i < 32; ++i) {
		address[i + i / 4] = reverse_name[62 - 2 * i];
		if (((i + 1) % 4)==0) address[i + i / 4 +1] = ':';
	}
	address[39] = 0;

	if (inet_pton(AF_INET6, address, (void *)dst) == 1)
		return true;
	
	return false;
} 

bool create_v4_reverse_name(char *dst, in_addr *v4_address)
{
	u8 next_byte;
	u8 *data = (u8*)v4_address;

	dst[0] = 0;

	for (u32 i(0); i < sizeof(in_addr); ++i) {
		next_byte = data[sizeof(in_addr) - 1 - i];
		sprintf(dst + strlen(dst), "%d.", next_byte);
	}

	strcat(dst, "in-addr.arpa");

	return true;
}

bool create_v6_reverse_name(char *dst, in6_addr *v6_address)
{
	unsigned char next_byte;

	for (u32 i(0); i < 16; ++i) {
		next_byte = v6_address->in6_u.u6_addr8[15 - i];
		sprintf(dst + 4 * i, "%x.%x.", next_byte & 0x0f, next_byte >> 4);
	}

	strcat(dst, "IP6.INT");
	
	return true;
}
