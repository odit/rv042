/***************************************************************************
 *  dns_helper.h : This file is part of 'ataga'
 *  created on: Thu Jun 16 16:42:34 CDT 2005
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

#ifndef __DNS_HELPER__H_
#define __DNS_HELPER__H_

#include "ataga.h"

void *dns_helper_init(void*);
bool Add_DNS_Request(in_addr*, NAT_Manager::protocol_type, pending_packet*);
bool Add_DNS_Request(DNS_find*);
bool Remove_DNS_Request(DNS_find *request);
bool FindDNSMatch(DNS_find *request);

#endif // __DNS_HELPER__H_
