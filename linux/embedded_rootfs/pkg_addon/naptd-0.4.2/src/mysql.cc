/***************************************************************************
 *  mysql.cc : This file is part of 'ataga'
 *  created on: Sat Jul 30 22:20:36 CDT 2005
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
 
#include "mysql.h"
#include "ataga.h"

MYSQL *mysql_con;

bool CloseMysql()
{
	mysql_close(mysql_con);
	return true;
}

bool InitMysql(const char *host, const char *user, const char *passwd, const char *db)
{
	mysql_con = mysql_init(0);
	
	if (!mysql_con) {
		pLog->write("Unable to init MySQL.\n");
		return false;
	}
	
	mysql_con = mysql_real_connect(mysql_con, host, user, passwd, db, 0, 0, 0);
	
	if (!mysql_con) {
		pLog->write("Unable to connect to MySQL database.\n");
		return false;
	}
	
	return true;
}
