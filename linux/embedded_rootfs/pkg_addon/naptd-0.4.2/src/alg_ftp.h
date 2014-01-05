/***************************************************************************
 *  alg_ftp.h : This file is part of 'ataga'
 *  created on: Mon Jun 20 14:22:16 CDT 2005
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

#ifndef __ALG_FTP_H__
#define __ALG_FTP_H__

bool v4_main(NAT_Manager::addr_mapping *mapping, char *buffer, u32 *size);
bool v6_main(NAT_Manager::addr_mapping *mapping, char *buffer, u32 *size);
const char *name_func(int code);
u16 port_func(int code);

bool FTP_PORT(NAT_Manager::addr_mapping*, char*, u32*, char*, u32*);
bool FTP_EPRT(NAT_Manager::addr_mapping*, char*, u32*, char*, u32*);
bool FTP_PASV(NAT_Manager::addr_mapping*, char*, u32*, char*, u32*);
bool FTP_EPSV(NAT_Manager::addr_mapping*, char*, u32*, char*, u32*);
bool FTP_227(NAT_Manager::addr_mapping*, char*, u32*, char*, u32*);
bool FTP_150_v4(NAT_Manager::addr_mapping*, char*, u32*, char*, u32*);
bool FTP_150_v6(NAT_Manager::addr_mapping*, char*, u32*, char*, u32*);

#endif // __ALG_FTP_H__
