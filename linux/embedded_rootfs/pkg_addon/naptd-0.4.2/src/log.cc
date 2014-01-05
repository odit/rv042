/***************************************************************************
 *  log.cc : This file is part of 'ataga'
 *  created on: Fri Apr 22 16:31:25 CDT 2005
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

#include <time.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>
#include "log.h"

CLog::CLog(bool where) : std(where)
{
	if (!std)
		openlog("naptd", LOG_PID, LOG_DAEMON);
    else
		hFile = stdout;
	
	/* revised as of version 0.2 */
	/*
	hFile = 0;
	
	const time_t s(time(0));
	struct tm *t= localtime(&s);
		
	logfile_path = new char[1024];
	snprintf(logfile_path, 1024, "%s%02d.%02d.%02d.%02d.%02d.%02d.log", path, 
		1900 + t->tm_year, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
	
	if (!append_mode) {
		hFile = fopen(logfile_path, "w+");
		Close();
		
	//	 SPECIAL START 
	//	 added especially for ataga 
	//	 consider adding to base class 
		
		int fd = open(logfile_path, O_RDONLY);
		fchown(fd, getpid() + 1, getpid() + 1);
		close(fd);
		
	//	 SPECIAL END 
	}
	*/
}

CLog::~CLog()
{
	if (!std) 
		closelog();
}

void CLog::write(char *text,...)
{
	va_list arglist;
	char buffer[1024], buffer2[1024], time_buffer[256];
	memset(buffer, 0, sizeof(buffer));

	va_start(arglist, text);
	vsnprintf(buffer, 1024, text, arglist);
	va_end(arglist);
	
	if (!std) {
		syslog(LOG_WARNING, buffer);
		return;
	}
	
	const time_t s(time(0));
	strftime(time_buffer, 256, "%A, %d %B %Y %H:%M:%S", localtime(&s));
	snprintf(buffer2, 1024, "%s : %s\n", time_buffer, buffer);
	
	fprintf(hFile, buffer2);
}
