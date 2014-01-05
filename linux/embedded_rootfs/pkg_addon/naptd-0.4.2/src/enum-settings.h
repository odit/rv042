/***************************************************************************
 *  enum-settings.h : This file is part of 'ataga'
 *  created on: Thu Jun  9 10:56:30 CDT 2005
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

#ifndef __ENUM_SETTINGS_ATAGA_H_
#define __ENUM_SETTINGS_ATAGA_H_

/* enums for CSettings */

enum {
	number_of_static_mappings = 0,
	number_of_public_ip_blocks,
	number_of_priv_blocks,
	number_of_inside_interfaces,
	number_of_outside_interfaces,
	get_v4_from_outside_int,
	translation_time_tcp,
	translation_time_udp,
	translation_time_icmp
};

enum {
	ipv6_prefix = 0
};

#endif // __ENUM_SETTINGS_ATAGA_H_
