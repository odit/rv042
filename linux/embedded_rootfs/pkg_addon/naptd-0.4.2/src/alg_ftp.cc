/***************************************************************************
 *  alg_ftp.cc : This file is part of 'ataga'
 *  created on: Thu Jun 16 16:45:01 CDT 2005
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
 
#include "ataga.h"
#include "alg.h"
#include "alg_ftp.h"

in6_addr v6_prefix;
bool (*seq_num)(NAT_Manager::addr_mapping*, int*, int*, u16, seq_action);

bool v4_main(NAT_Manager::addr_mapping *mapping, char *buffer, u32 *size)
{
	iphdr *ip = (iphdr*)buffer;
	
	if (ip->protocol != IPPROTO_TCP)
		return true;
	
	tcphdr *tcp = (tcphdr*)(buffer + ip->ihl * 4);
	u32 ftp_offset = (tcp->doff * 4);
	u32 payload_size = htons(ip->tot_len) - ip->ihl * 4;
	
	int seq(0);
	int ack_seq(0);
	
	(*seq_num)(mapping, &seq, &ack_seq, IPPROTO_IP, seq_get);
	
	tcp->seq = ntohl(htonl(tcp->seq) + seq);
	tcp->ack_seq = ntohl(htonl(tcp->ack_seq) - ack_seq);
	
	if (ftp_offset < payload_size) {
		u32 ftp_data_size = payload_size - ftp_offset;
		
		/* get command */
		char ftp_command[50];
		char *p = (buffer + ftp_offset + ip->ihl * 4);
		u32 i(0);
		for (; p[i] != '\n' && p[i] != '\r' && p[i] != ' ' && i < ftp_data_size && i < 50; ++i)
			ftp_command[i] = p[i];
		
		ftp_command[i] = 0;
		/* end get command */
		/* the above code could be made a separate function, but one must  		*/
		/* consider thread safety in the context of providing return data, as  	*/
		/* well as performance because allocating new memory blocks per tiny  	*/
		/* command may not be the best thing to do 								*/

#ifdef _DEBUG_
		
		printf("got FTP command %s\n", ftp_command);
		
#endif // _DEBUG_
		
		if (!strcmp(ftp_command, "229")) {
		} else if (!strcmp(ftp_command, "150")) {
			return FTP_150_v4(mapping, p, &ftp_data_size, buffer, size);
		} else if (!strcmp(ftp_command, "EPRT")) {
			return FTP_EPRT(mapping, p, &ftp_data_size, buffer, size);
		} else if (!strcmp(ftp_command, "EPSV")) {
			return FTP_EPSV(mapping, p, &ftp_data_size, buffer, size);
		}
	}
	
	return true;
}

bool v6_main(NAT_Manager::addr_mapping *mapping, char *buffer, u32 *size)
{
	ip6_hdr *ip6 = (ip6_hdr*)buffer;
	
	if (ip6->ip6_nxt != IPPROTO_TCP)
		return true;
	
	tcphdr *tcp = (tcphdr*)(buffer + sizeof(ip6_hdr));
	u32 ftp_offset = (tcp->doff * 4);
	u32 payload_size = htons(ip6->ip6_plen);
	
	int seq(0);
	int ack_seq(0);
	
	(*seq_num)(mapping, &seq, &ack_seq, IPPROTO_IPV6, seq_get);
	
	tcp->seq = ntohl(htonl(tcp->seq) + seq);
	tcp->ack_seq = ntohl(htonl(tcp->ack_seq) - ack_seq);
	
	if (ftp_offset < payload_size) {
		u32 ftp_data_size = payload_size - ftp_offset;
		
		/* get command */
		char ftp_command[50];
		char *p = (buffer + ftp_offset + sizeof(ip6_hdr));
		u32 i(0);
		for (; p[i] != '\n' && p[i] != '\r' && p[i] != ' ' && i < ftp_data_size && i < 50; ++i)
			ftp_command[i] = p[i];
		
		ftp_command[i] = 0;
		/* end get command */
		/* the above code could be made a separate function, but one must  		*/
		/* consider thread safety in the context of providing return data, as  	*/
		/* well as performance because allocating new memory blocks per tiny  	*/
		/* command may not be the best thing to do 								*/

#ifdef _DEBUG_
		
		printf("got FTP command %s\n", ftp_command);
		
#endif // _DEBUG_
		
		if (!strcmp(ftp_command, "PORT")) {
			return FTP_PORT(mapping, p, &ftp_data_size, buffer, size);
		} else if (!strcmp(ftp_command, "200")) {
			//return FTP_200(mapping, ftp_command, &ftp_data_size, buffer, size);
		} else if (!strcmp(ftp_command, "PASV")) {
			return FTP_PASV(mapping, p, &ftp_data_size, buffer, size);
		} else if (!strcmp(ftp_command, "221")) {
		} else if (!strcmp(ftp_command, "150")) {
			return FTP_150_v6(mapping, p, &ftp_data_size, buffer, size);
		} else if (!strcmp(ftp_command, "227")) {
			return FTP_227(mapping, p, &ftp_data_size, buffer, size);
		}
	}

	return true;	
}

bool FTP_PASV(NAT_Manager::addr_mapping *mapping, char *command, u32 *data_length, char *buffer, u32 *size)
{
	ip6_hdr *ip6 = (ip6_hdr*)(buffer);
	tcphdr *tcp = (tcphdr*)(buffer + sizeof(ip6_hdr));

	char *new_command = "EPSV\r\n";
	strcpy(buffer + tcp->doff * 4 + sizeof(ip6_hdr), new_command);
		
	int new_payload_length(strlen(new_command));
	
	*size = sizeof(ip6_hdr) + tcp->doff * 4 + new_payload_length;
	ip6->ip6_plen = htons(*size - sizeof(ip6_hdr));
	
	int total_payload_offset(new_payload_length - *data_length);
	
#ifdef _DEBUG_
	
	int seq(0);
	int ack_seq(0);
	(*seq_num)(mapping, &seq, &ack_seq, IPPROTO_IPV6, seq_get);
	printf("old length(%u) new length(%u) offset(%i)\n", *data_length, new_payload_length, seq + total_payload_offset);

#endif // _DEBUG_
	
	if (total_payload_offset)
		(*seq_num)(mapping, &total_payload_offset, 0, IPPROTO_IPV6, seq_set);
	
	return true;
}

bool FTP_EPSV(NAT_Manager::addr_mapping *mapping, char *command, u32 *data_length, char *buffer, u32 *size)
{
	iphdr *ip = (iphdr*)(buffer);
	tcphdr *tcp = (tcphdr*)(buffer + ip->ihl * 4);

	char *new_command = "PASV\r\n";
	strcpy(buffer + tcp->doff * 4 + ip->ihl * 4, new_command);
		
	int new_payload_length(strlen(new_command));
	
	*size = ip->ihl * 4 + tcp->doff * 4 + new_payload_length;
	ip->tot_len = htons(*size);
	
	int total_payload_offset(new_payload_length - *data_length);
	
#ifdef _DEBUG_
	
	int seq(0);
	int ack_seq(0);
	(*seq_num)(mapping, &seq, &ack_seq, IPPROTO_IP, seq_get);
	printf("old length(%u) new length(%u) offset(%i)\n", *data_length, new_payload_length, seq + total_payload_offset);

#endif // _DEBUG_
	
	if (total_payload_offset)
		(*seq_num)(mapping, &total_payload_offset, 0, IPPROTO_IP, seq_set);
	
	return true;
}

bool FTP_EPRT(NAT_Manager::addr_mapping *mapping, char *command, u32 *data_length, char *buffer, u32 *size)
{
	return true;

	
//	ip6_hdr *ip6 = (ip6_hdr*)(buffer);
//	tcphdr *tcp = (tcphdr*)(buffer + sizeof(ip6_hdr));
	
//	strcpy(buffer + sizeof(ip6_hdr) + tcp->doff * 4, new_ftp_command);
//	
//	int new_payload_length(strlen(new_ftp_command));
//	
//	*size = sizeof(ip6_hdr) + tcp->doff * 4 + new_payload_length;
//	ip6->ip6_plen = htons(*size - sizeof(ip6_hdr));
//		
//	int total_payload_offset(new_payload_length - *data_length);
//	
//#ifdef _DEBUG_
//	
//	int seq(0);
//	int ack_seq(0);
//	(*seq_num)(mapping, &seq, &ack_seq, IPPROTO_IP, seq_get);
//	printf("old length(%u) new length(%u) offset(%i)\n", *data_length, new_payload_length, seq + total_payload_offset);
//
//#endif // _DEBUG_
//	
//	if (total_payload_offset)
//		(*seq_num)(mapping, &total_payload_offset, 0, IPPROTO_IP, seq_set);
//	
//	
//	return false;
}

bool FTP_227(NAT_Manager::addr_mapping *mapping, char *command, u32 *data_length, char *buffer, u32 *size)
{
	u32 ftp_element_count(4), number_of_commas(0);

	while (ftp_element_count < *data_length && number_of_commas < 4) {
		if (command[ftp_element_count] == ',')
			++number_of_commas;
		
		++ftp_element_count;
	}

	char port_char[10];
	u32 port_count(0), port(0), port1(0), port2(0);

	while (port_count < 10 && ftp_element_count < *data_length && command[ftp_element_count] != ')') {
		if (command[ftp_element_count] == ',') {
			port_char[port_count] = 0;
			port1 = atoi(port_char);
			port1 = port1 << 8;
			port_count = 0;
		} else {
			port_char[port_count] = command[ftp_element_count];
			++port_count;
		}
		++ftp_element_count;
	}
	if (port_count < 10) {
		port_char[port_count] = 0;
		port2 = atoi(port_char);
	}
	port = port1 + port2;
	
	char new_ftp_command[100];
	strcpy(new_ftp_command, "229 Entering Extended Passive Mode (|||");
	sprintf(port_char,"%d", port);
	strcat(new_ftp_command, port_char);
	strcat(new_ftp_command,"|)\r\n");
	
	ip6_hdr *ip6 = (ip6_hdr*)(buffer);
	tcphdr *tcp = (tcphdr*)(buffer + sizeof(ip6_hdr));
	
	strcpy(buffer + sizeof(ip6_hdr) + tcp->doff * 4, new_ftp_command);
	
	int new_payload_length(strlen(new_ftp_command));
	
	*size = sizeof(ip6_hdr) + tcp->doff * 4 + new_payload_length;
	ip6->ip6_plen = htons(*size - sizeof(ip6_hdr));
		
	int total_payload_offset(new_payload_length - *data_length);
	
#ifdef _DEBUG_
	
	int seq(0);
	int ack_seq(0);
	(*seq_num)(mapping, &seq, &ack_seq, IPPROTO_IPV6, seq_get);
	printf("old length(%u) new length(%u) offset(%i)\n", *data_length, new_payload_length, seq + total_payload_offset);

#endif // _DEBUG_
	
	if (total_payload_offset)
		(*seq_num)(mapping, &total_payload_offset, 0, IPPROTO_IPV6, seq_set);
	
	return true;
}

bool FTP_150_v4(NAT_Manager::addr_mapping *mapping, char *command, u32 *data_length, char *buffer, u32 *size) 
{
	iphdr *ip = (iphdr*)(buffer);
	tcphdr *tcp = (tcphdr*)(buffer + ip->ihl * 4);

	char *new_command = "150 ASCII data connection for /bin/ls\r\n";
	strcpy(buffer + tcp->doff * 4 + ip->ihl * 4, new_command);
		
	int new_payload_length(strlen(new_command));
	
	*size = ip->ihl * 4 + tcp->doff * 4 + new_payload_length;
	ip->tot_len = htons(*size);
	
	int total_payload_offset(new_payload_length - *data_length);
	
#ifdef _DEBUG_
	
	int seq(0);
	int ack_seq(0);
	(*seq_num)(mapping, &seq, &ack_seq, IPPROTO_IP, seq_get);
	printf("old length(%u) new length(%u) offset(%i)\n", *data_length, new_payload_length, seq + total_payload_offset);

#endif // _DEBUG_
	
	if (total_payload_offset)
		(*seq_num)(mapping, &total_payload_offset, 0, IPPROTO_IP, seq_set);
	
	return true;
}

bool FTP_150_v6(NAT_Manager::addr_mapping *mapping, char *command, u32 *data_length, char *buffer, u32 *size) 
{
	ip6_hdr *ip6 = (ip6_hdr*)(buffer);
	tcphdr *tcp = (tcphdr*)(buffer + sizeof(ip6_hdr));

	char *new_command = "150 ASCII data connection for /bin/ls\r\n";
	strcpy(buffer + tcp->doff * 4 + sizeof(ip6_hdr), new_command);
		
	int new_payload_length(strlen(new_command));
	
	*size = sizeof(ip6_hdr) + tcp->doff * 4 + new_payload_length;
	ip6->ip6_plen = htons(*size - sizeof(ip6_hdr));
	
	int total_payload_offset(new_payload_length - *data_length);
	
#ifdef _DEBUG_
	
	int seq(0);
	int ack_seq(0);
	(*seq_num)(mapping, &seq, &ack_seq, IPPROTO_IPV6, seq_get);
	printf("old length(%u) new length(%u) offset(%i)\n", *data_length, new_payload_length, seq + total_payload_offset);

#endif // _DEBUG_
	
	if (total_payload_offset)
		(*seq_num)(mapping, &total_payload_offset, 0, IPPROTO_IPV6, seq_set);
	
	return true;
}

bool FTP_PORT(NAT_Manager::addr_mapping *mapping, char *command, u32 *data_length, char *buffer, u32 *size) 
{	
	u32 ftp_element_count(5), number_of_commas(0), address_count(0);
	char ip_address[15];
	
	while (ftp_element_count < *data_length && number_of_commas < 4) {
		if (command[ftp_element_count] == ',') {
			if (number_of_commas != 3) {
				ip_address[address_count] = '.';
				++address_count;
			}
			++number_of_commas;
		} else {
			ip_address[address_count] = command[ftp_element_count];
			++address_count;
		}
		++ftp_element_count;
	}
	ip_address[address_count] = 0;

	char port_char[10];
	u32 port_count(0), port(0), port1(0), port2(0);
	
	while (ftp_element_count < *data_length && command[ftp_element_count] != '\r') {
		if (command[ftp_element_count] == ',') {
			port_char[port_count] = 0;
			port1 = atoi(port_char);
			port1 = port1 << 8;
			port_count = 0;
		} else {
			port_char[port_count] = command[ftp_element_count];
			++port_count;		
		}
			++ftp_element_count;	
	}
	if (port_count < 10) {
		port_char[port_count] = 0;
		port2 = atoi(port_char);
	}
	port = port1 + port2;

	char ip6_address[INET6_ADDRSTRLEN];
	in6_addr v6_addr;
	memcpy(&v6_addr, &v6_prefix, 12);
	
	inet_ntop(AF_INET, ip_address, (char*)&v6_addr + 12, INET_ADDRSTRLEN);
	inet_ntop(AF_INET6, &v6_addr, ip6_address, INET6_ADDRSTRLEN);

	char new_ftp_command[100];
	strcpy(new_ftp_command,"EPRT |2|");
	strcat(new_ftp_command,ip6_address);
	strcat(new_ftp_command,"|");
	sprintf(port_char,"%d", port);
	strcat(new_ftp_command, port_char);
	strcat(new_ftp_command,"|");
	strcat(new_ftp_command,"\r\n");
	
	ip6_hdr *ip6 = (ip6_hdr*)(buffer);
	tcphdr *tcp = (tcphdr*)(buffer + sizeof(ip6_hdr));
	
	strcpy(buffer + sizeof(ip6_hdr) + tcp->doff * 4, new_ftp_command);
	
	int new_payload_length(strlen(new_ftp_command));
	
	*size = sizeof(ip6_hdr) + tcp->doff * 4 + new_payload_length;
	ip6->ip6_plen = htons(*size - sizeof(ip6_hdr));
		
	int total_payload_offset(new_payload_length - *data_length);
	
#ifdef _DEBUG_
	
	int seq(0);
	int ack_seq(0);
	(*seq_num)(mapping, &seq, &ack_seq, IPPROTO_IPV6, seq_get);
	printf("old length(%u) new length(%u) offset(%i)\n", *data_length, new_payload_length, seq + total_payload_offset);

#endif // _DEBUG_
	
	if (total_payload_offset)
		(*seq_num)(mapping, &total_payload_offset, 0, IPPROTO_IPV6, seq_set);
	
	return true;
}

const char *name_func(int code)
{
	static char name[] = "File Transfer Protocol";
	
	if (code)
		memcpy(&v6_prefix, (void*)code, sizeof(in6_addr));
	
	return name;
}

u16 port_func(int code)
{
	static u16 port(21);
	
	return port;
}

bool init_plugin(bool (*p)(NAT_Manager::addr_mapping*, int*, int*, u16, seq_action))
{
	seq_num = p;
	
	return true;
}
