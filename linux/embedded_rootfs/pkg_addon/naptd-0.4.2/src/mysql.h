/***************************************************************************
 *  mysql.h : This file is part of 'ataga'
 *  created on: Sat Jul 30 22:21:21 CDT 2005
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

#ifndef __MYSQL_H_
#define __MYSQL_H_

#include <mysql/mysql.h>

bool InitMysql(const char *host, const char *user, const char *passwd, const char *db);
bool CloseMysql();

extern MYSQL *mysql_con;

#endif // __MYSQL_H_
