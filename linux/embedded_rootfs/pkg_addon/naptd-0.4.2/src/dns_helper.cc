/***************************************************************************
 *  dns_helper.cc : This file is part of 'ataga'
 *  created on: Thu Jun 16 16:43:26 CDT 2005
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
#include "dns_helper.h"
#include "nat-mngr.h"

pthread_cond_t 		dns_helper_thread_wait;
DNS_find 			*dns_request_list = 0;

void *dns_helper_init(void *p) {
	/* main loop */
	while (true) {
		DNS_find *i = dns_request_list;
		
		while (i) {
			if (i->packet->status != awaiting_DNS_request) {
				Remove_DNS_Request(i);
				i = dns_request_list;
			} else {
				if (i->packet->status == awaiting_DNS_request)
					if (FindDNSMatch(i))
						i->packet->status = ready_for_processing;
					else
						i->packet->status = ready_for_discarding;
				i = i->next;
			}
		}
		
		pthread_cond_init(&dns_helper_thread_wait, 0);
		pthread_mutex_t wait_mutex = PTHREAD_MUTEX_INITIALIZER;
		pthread_mutex_lock(&wait_mutex);
		pthread_cond_wait(&dns_helper_thread_wait, &wait_mutex);
		pthread_mutex_unlock(&wait_mutex);
	}
	
	return 0;
}

bool Remove_DNS_Request(DNS_find *request)
{
	dns_request_list = request->next;
	delete request;
	
	return true;
}

bool Add_DNS_Request(DNS_find *request)
{
	if (!dns_request_list)
		dns_request_list = request;
	else {
		request->next = dns_request_list;
		dns_request_list = request;
	}
	
	pthread_cond_signal(&dns_helper_thread_wait);
	
	return true;
}

bool Add_DNS_Request(in_addr *addr, NAT_Manager::protocol_type prot, pending_packet *packet) 
{
	DNS_find *request = new DNS_find;
	
	memcpy(&request->addr, addr, sizeof(in_addr));
	request->prot = prot;
	request->packet = packet;
	request->next = 0;
	
	return Add_DNS_Request(request);
}

bool FindDNSMatch(DNS_find *request)
{	
	hostent *h = gethostbyaddr((const char*) &request->addr, 4, AF_INET);
	
	if (!h) {
		return false;
	}

	addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_protocol = AF_INET6;
	
	addrinfo *res, *resave;
	
	if (int r = getaddrinfo(h->h_name, 0, &hints, &res)) {
		pLog->write("error while looking for AAAA record for the hostname %s: ", h->h_name, gai_strerror(r));
		res = 0;
		return false;
	}
	
	bool addr_found(false);
	resave = res;
	sockaddr_in6 sock_addr;
	
	while (res) {
		if (res->ai_family == AF_INET6) {
			memcpy(&sock_addr, res->ai_addr, res->ai_addrlen);
			addr_found = true;
			break;
		}
		res = res->ai_next;
	}
	freeaddrinfo(resave);
	
	if (!addr_found)
		return false;
	
	pNat->Add_Dynamic_Mapping(&request->addr, request->prot, &sock_addr.sin6_addr);
	
	return true;
}
