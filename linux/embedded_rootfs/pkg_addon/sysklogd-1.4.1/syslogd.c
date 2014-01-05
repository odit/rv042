/*
 * Copyright (c) 1983, 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#if !defined(lint) && !defined(NO_SCCS)
char copyright2[] =
"@(#) Copyright (c) 1983, 1988 Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#if !defined(lint) && !defined(NO_SCCS)
static char sccsid[] = "@(#)syslogd.c	5.27 (Berkeley) 10/10/88";
#endif /* not lint */

/*
 *  syslogd -- log system messages
 *
 * This program implements a system log. It takes a series of lines.
 * Each line may have a priority, signified as "<n>" as
 * the first characters of the line.  If this is
 * not present, a default priority is used.
 *
 * To kill syslogd, send a signal 15 (terminate).  A signal 1 (hup) will
 * cause it to reread its configuration file.
 *
 * Defined Constants:
 *
 * MAXLINE -- the maximum line length that can be handled.
 * DEFUPRI -- the default priority for user messages
 * DEFSPRI -- the default priority for kernel messages
 *
 * Author: Eric Allman
 * extensive changes by Ralph Campbell
 * more extensive changes by Eric Allman (again)
 *
 * Steve Lord:	Fix UNIX domain socket code, added linux kernel logging
 *		change defines to
 *		SYSLOG_INET	- listen on a UDP socket
 *		SYSLOG_UNIXAF	- listen on unix domain socket
 *		SYSLOG_KERNEL	- listen to linux kernel
 *
 * Mon Feb 22 09:55:42 CST 1993:  Dr. Wettstein
 * 	Additional modifications to the source.  Changed priority scheme
 *	to increase the level of configurability.  In its stock configuration
 *	syslogd no longer logs all messages of a certain priority and above
 *	to a log file.  The * wildcard is supported to specify all priorities.
 *	Note that this is a departure from the BSD standard.
 *
 *	Syslogd will now listen to both the inetd and the unixd socket.  The
 *	strategy is to allow all local programs to direct their output to
 *	syslogd through the unixd socket while the program listens to the
 *	inetd socket to get messages forwarded from other hosts.
 *
 * Fri Mar 12 16:55:33 CST 1993:  Dr. Wettstein
 *	Thanks to Stephen Tweedie (dcs.ed.ac.uk!sct) for helpful bug-fixes
 *	and an enlightened commentary on the prioritization problem.
 *
 *	Changed the priority scheme so that the default behavior mimics the
 *	standard BSD.  In this scenario all messages of a specified priority
 *	and above are logged.
 *
 *	Add the ability to specify a wildcard (=) as the first character
 *	of the priority name.  Doing this specifies that ONLY messages with
 *	this level of priority are to be logged.  For example:
 *
 *		*.=debug			/usr/adm/debug
 *
 *	Would log only messages with a priority of debug to the /usr/adm/debug
 *	file.
 *
 *	Providing an * as the priority specifies that all messages are to be
 *	logged.  Note that this case is degenerate with specifying a priority
 *	level of debug.  The wildcard * was retained because I believe that
 *	this is more intuitive.
 *
 * Thu Jun 24 11:34:13 CDT 1993:  Dr. Wettstein
 *	Modified sources to incorporate changes in libc4.4.  Messages from
 *	syslog are now null-terminated, syslogd code now parses messages
 *	based on this termination scheme.  Linux as of libc4.4 supports the
 *	fsync system call.  Modified code to fsync after all writes to
 *	log files.
 *
 * Sat Dec 11 11:59:43 CST 1993:  Dr. Wettstein
 *	Extensive changes to the source code to allow compilation with no
 *	complaints with -Wall.
 *
 *	Reorganized the facility and priority name arrays so that they
 *	compatible with the syslog.h source found in /usr/include/syslog.h.
 *	NOTE that this should really be changed.  The reason I do not
 *	allow the use of the values defined in syslog.h is on account of
 *	the extensions made to allow the wildcard character in the
 *	priority field.  To fix this properly one should malloc an array,
 *	copy the contents of the array defined by syslog.h and then
 *	make whatever modifications that are desired.  Next round.
 *
 * Thu Jan  6 12:07:36 CST 1994:  Dr. Wettstein
 *	Added support for proper decomposition and re-assembly of
 *	fragment messages on UNIX domain sockets.  Lack of this capability
 *	was causing 'partial' messages to be output.  Since facility and
 *	priority information is encoded as a leader on the messages this
 *	was causing lines to be placed in erroneous files.
 *
 *	Also added a patch from Shane Alderton (shane@ion.apana.org.au) to
 *	correct a problem with syslogd dumping core when an attempt was made
 *	to write log messages to a logged-on user.  Thank you.
 *
 *	Many thanks to Juha Virtanen (jiivee@hut.fi) for a series of
 *	interchanges which lead to the fixing of problems with messages set
 *	to priorities of none and emerg.  Also thanks to Juha for a patch
 *	to exclude users with a class of LOGIN from receiving messages.
 *
 *	Shane Alderton provided an additional patch to fix zombies which
 *	were conceived when messages were written to multiple users.
 *
 * Mon Feb  6 09:57:10 CST 1995:  Dr. Wettstein
 *	Patch to properly reset the single priority message flag.  Thanks
 *	to Christopher Gori for spotting this bug and forwarding a patch.
 *
 * Wed Feb 22 15:38:31 CST 1995:  Dr. Wettstein
 *	Added version information to startup messages.
 *
 *	Added defines so that paths to important files are taken from
 *	the definitions in paths.h.  Hopefully this will insure that
 *	everything follows the FSSTND standards.  Thanks to Chris Metcalf
 *	for a set of patches to provide this functionality.  Also thanks
 *	Elias Levy for prompting me to get these into the sources.
 *
 * Wed Jul 26 18:57:23 MET DST 1995:  Martin Schulze
 *	Linux' gethostname only returns the hostname and not the fqdn as
 *	expected in the code. But if you call hostname with an fqdn then
 *	gethostname will return an fqdn, so we have to mention that. This
 *	has been changed.
 *
 *	The 'LocalDomain' and the hostname of a remote machine is
 *	converted to lower case, because the original caused some
 *	inconsistency, because the (at least my) nameserver did respond an
 *	fqdn containing of upper- _and_ lowercase letters while
 *	'LocalDomain' consisted only of lowercase letters and that didn't
 *	match.
 *
 * Sat Aug  5 18:59:15 MET DST 1995:  Martin Schulze
 *	Now no messages that were received from any remote host are sent
 *	out to another. At my domain this missing feature caused ugly
 *	syslog-loops, sometimes.
 *
 *	Remember that no message is sent out. I can't figure out any
 *	scenario where it might be useful to change this behavior and to
 *	send out messages to other hosts than the one from which we
 *	received the message, but I might be shortsighted. :-/
 *
 * Thu Aug 10 19:01:08 MET DST 1995:  Martin Schulze
 *	Added my pidfile.[ch] to it to perform a better handling with
 *	pidfiles. Now both, syslogd and klogd, can only be started
 *	once. They check the pidfile.
 *
 * Sun Aug 13 19:01:41 MET DST 1995:  Martin Schulze
 *	Add an addition to syslog.conf's interpretation. If a priority
 *	begins with an exclamation mark ('!') the normal interpretation
 *	of the priority is inverted: ".!*" is the same as ".none", ".!=info"
 *	don't logs the info priority, ".!crit" won't log any message with
 *	the priority crit or higher. For example:
 *
 *		mail.*;mail.!=info		/usr/adm/mail
 *
 *	Would log all messages of the facility mail except those with
 *	the priority info to /usr/adm/mail. This makes the syslogd
 *	much more flexible.
 *
 *	Defined TABLE_ALLPRI=255 and changed some occurrences.
 *
 * Sat Aug 19 21:40:13 MET DST 1995:  Martin Schulze
 *	Making the table of facilities and priorities while in debug
 *	mode more readable.
 *
 *	If debugging is turned on, printing the whole table of
 *	facilities and priorities every hexadecimal or 'X' entry is
 *	now 2 characters wide.
 *
 *	The number of the entry is prepended to each line of
 *	facilities and priorities, and F_UNUSED lines are not shown
 *	anymore.
 *
 *	Corrected some #ifdef SYSV's.
 *
 * Mon Aug 21 22:10:35 MET DST 1995:  Martin Schulze
 *	Corrected a strange behavior during parsing of configuration
 *	file. The original BSD syslogd doesn't understand spaces as
 *	separators between specifier and action. This syslogd now
 *	understands them. The old behavior caused some confusion over
 *	the Linux community.
 *
 * Thu Oct 19 00:02:07 MET 1995:  Martin Schulze
 *	The default behavior has changed for security reasons. The
 *	syslogd will not receive any remote message unless you turn
 *	reception on with the "-r" option.
 *
 *	Not defining SYSLOG_INET will result in not doing any network
 *	activity, i.e. not sending or receiving messages.  I changed
 *	this because the old idea is implemented with the "-r" option
 *	and the old thing didn't work anyway.
 *
 * Thu Oct 26 13:14:06 MET 1995:  Martin Schulze
 *	Added another logfile type F_FORW_UNKN.  The problem I ran into
 *	was a name server that runs on my machine and a forwarder of
 *	kern.crit to another host.  The hosts address can only be
 *	fetched using the nameserver.  But named is started after
 *	syslogd, so syslogd complained.
 *
 *	This logfile type will retry to get the address of the
 *	hostname ten times and then complain.  This should be enough to
 *	get the named up and running during boot sequence.
 *
 * Fri Oct 27 14:08:15 1995:  Dr. Wettstein
 *	Changed static array of logfiles to a dynamic array. This
 *	can grow during process.
 *
 * Fri Nov 10 23:08:18 1995:  Martin Schulze
 *	Inserted a new tabular sys_h_errlist that contains plain text
 *	for error codes that are returned from the net subsystem and
 *	stored in h_errno. I have also changed some wrong lookups to
 *	sys_errlist.
 *
 * Wed Nov 22 22:32:55 1995:  Martin Schulze
 *	Added the fabulous strip-domain feature that allows us to
 *	strip off (several) domain names from the fqdn and only log
 *	the simple hostname. This is useful if you're in a LAN that
 *	has a central log server and also different domains.
 *
 *	I have also also added the -l switch do define hosts as
 *	local. These will get logged with their simple hostname, too.
 *
 * Thu Nov 23 19:02:56 MET DST 1995:  Martin Schulze
 *	Added the possibility to omit fsyncing of logfiles after every
 *	write. This will give some performance back if you have
 *	programs that log in a very verbose manner (like innd or
 *	smartlist). Thanks to Stephen R. van den Berg <srb@cuci.nl>
 *	for the idea.
 *
 * Thu Jan 18 11:14:36 CST 1996:  Dr. Wettstein
 *	Added patche from beta-testers to stop compile error.  Also
 *	added removal of pid file as part of termination cleanup.
 *
 * Wed Feb 14 12:42:09 CST 1996:  Dr. Wettstein
 *	Allowed forwarding of messages received from remote hosts to
 *	be controlled by a command-line switch.  Specifying -h allows
 *	forwarding.  The default behavior is to disable forwarding of
 *	messages which were received from a remote host.
 *
 *	Parent process of syslogd does not exit until child process has
 *	finished initialization process.  This allows rc.* startup to
 *	pause until syslogd facility is up and operating.
 *
 *	Re-arranged the select code to move UNIX domain socket accepts
 *	to be processed later.  This was a contributed change which
 *	has been proposed to correct the delays sometimes encountered
 *	when syslogd starts up.
 *
 *	Minor code cleanups.
 *
 * Thu May  2 15:15:33 CDT 1996:  Dr. Wettstein
 *	Fixed bug in init function which resulted in file descripters
 *	being orphaned when syslogd process was re-initialized with SIGHUP
 *	signal.  Thanks to Edvard Tuinder
 *	(Edvard.Tuinder@praseodymium.cistron.nl) for putting me on the
 *	trail of this bug.  I am amazed that we didn't catch this one
 *	before now.
 *
 * Tue May 14 00:03:35 MET DST 1996:  Martin Schulze
 *	Corrected a mistake that causes the syslogd to stop logging at
 *	some virtual consoles under Linux. This was caused by checking
 *	the wrong error code. Thanks to Michael Nonweiler
 *	<mrn20@hermes.cam.ac.uk> for sending me a patch.
 *
 * Mon May 20 13:29:32 MET DST 1996:  Miquel van Smoorenburg <miquels@cistron.nl>
 *	Added continuation line supported and fixed a bug in
 *	the init() code.
 *
 * Tue May 28 00:58:45 MET DST 1996:  Martin Schulze
 *	Corrected behaviour of blocking pipes - i.e. the whole system
 *	hung.  Michael Nonweiler <mrn20@hermes.cam.ac.uk> has sent us
 *	a patch to correct this.  A new logfile type F_PIPE has been
 *	introduced.
 *
 * Mon Feb 3 10:12:15 MET DST 1997:  Martin Schulze
 *	Corrected behaviour of logfiles if the file can't be opened.
 *	There was a bug that causes syslogd to try to log into non
 *	existing files which ate cpu power.
 *
 * Sun Feb 9 03:22:12 MET DST 1997:  Martin Schulze
 *	Modified syslogd.c to not kill itself which confuses bash 2.0.
 *
 * Mon Feb 10 00:09:11 MET DST 1997:  Martin Schulze
 *	Improved debug code to decode the numeric facility/priority
 *	pair into textual information.
 *
 * Tue Jun 10 12:35:10 MET DST 1997:  Martin Schulze
 *	Corrected freeing of logfiles.  Thanks to Jos Vos <jos@xos.nl>
 *	for reporting the bug and sending an idea to fix the problem.
 *
 * Tue Jun 10 12:51:41 MET DST 1997:  Martin Schulze
 *	Removed sleep(10) from parent process.  This has caused a slow
 *	startup in former times - and I don't see any reason for this.
 *
 * Sun Jun 15 16:23:29 MET DST 1997: Michael Alan Dorman
 *	Some more glibc patches made by <mdorman@debian.org>.
 *
 * Thu Jan  1 16:04:52 CET 1998: Martin Schulze <joey@infodrom.north.de
 *	Applied patch from Herbert Thielen <Herbert.Thielen@lpr.e-technik.tu-muenchen.de>.
 *	This included some balance parentheses for emacs and a bug in
 *	the exclamation mark handling.
 *
 *	Fixed small bug which caused syslogd to write messages to the
 *	wrong logfile under some very rare conditions.  Thanks to
 *	Herbert Xu <herbert@gondor.apana.org.au> for fiddling this out.
 *
 * Thu Jan  8 22:46:35 CET 1998: Martin Schulze <joey@infodrom.north.de>
 *	Reworked one line of the above patch as it prevented syslogd
 *	from binding the socket with the result that no messages were
 *	forwarded to other hosts.
 *
 * Sat Jan 10 01:33:06 CET 1998: Martin Schulze <joey@infodrom.north.de>
 *	Fixed small bugs in F_FORW_UNKN meachanism.  Thanks to Torsten
 *	Neumann <torsten@londo.rhein-main.de> for pointing me to it.
 *
 * Mon Jan 12 19:50:58 CET 1998: Martin Schulze <joey@infodrom.north.de>
 *	Modified debug output concerning remote receiption.
 *
 * Mon Feb 23 23:32:35 CET 1998: Topi Miettinen <Topi.Miettinen@ml.tele.fi>
 *	Re-worked handling of Unix and UDP sockets to support closing /
 *	opening of them in order to have it open only if it is needed
 *	either for forwarding to a remote host or by receiption from
 *	the network.
 *
 * Wed Feb 25 10:54:09 CET 1998: Martin Schulze <joey@infodrom.north.de>
 *	Fixed little comparison mistake that prevented the MARK
 *	feature to work properly.
 *
 * Wed Feb 25 13:21:44 CET 1998: Martin Schulze <joey@infodrom.north.de>
 *	Corrected Topi's patch as it prevented forwarding during
 *	startup due to an unknown LogPort.
 *
 * Sat Oct 10 20:01:48 CEST 1998: Martin Schulze <joey@infodrom.north.de>
 *	Added support for TESTING define which will turn syslogd into
 *	stdio-mode used for debugging.
 *
 * Sun Oct 11 20:16:59 CEST 1998: Martin Schulze <joey@infodrom.north.de>
 *	Reworked the initialization/fork code.  Now the parent
 *	process activates a signal handler which the daughter process
 *	will raise if it is initialized.  Only after that one the
 *	parent process may exit.  Otherwise klogd might try to flush
 *	its log cache while syslogd can't receive the messages yet.
 *
 * Mon Oct 12 13:30:35 CEST 1998: Martin Schulze <joey@infodrom.north.de>
 *	Redirected some error output with regard to argument parsing to
 *	stderr.
 *
 * Mon Oct 12 14:02:51 CEST 1998: Martin Schulze <joey@infodrom.north.de>
 *	Applied patch provided vom Topi Miettinen with regard to the
 *	people from OpenBSD.  This provides the additional '-a'
 *	argument used for specifying additional UNIX domain sockets to
 *	listen to.  This is been used with chroot()'ed named's for
 *	example.  See for http://www.psionic.com/papers/dns.html
 *
 * Mon Oct 12 18:29:44 CEST 1998: Martin Schulze <joey@infodrom.north.de>
 *	Added `ftp' facility which was introduced in glibc version 2.
 *	It's #ifdef'ed so won't harm with older libraries.
 *
 * Mon Oct 12 19:59:21 MET DST 1998: Martin Schulze <joey@infodrom.north.de>
 *	Code cleanups with regard to bsd -> posix transition and
 *	stronger security (buffer length checking).  Thanks to Topi
 *	Miettinen <tom@medialab.sonera.net>
 *	. index() --> strchr()
 *	. sprintf() --> snprintf()
 *	. bcopy() --> memcpy()
 *	. bzero() --> memset()
 *	. UNAMESZ --> UT_NAMESIZE
 *	. sys_errlist --> strerror()
 *
 * Mon Oct 12 20:22:59 CEST 1998: Martin Schulze <joey@infodrom.north.de>
 *	Added support for setutent()/getutent()/endutend() instead of
 *	binary reading the UTMP file.  This is the the most portable
 *	way.  This allows /var/run/utmp format to change, even to a
 *	real database or utmp daemon. Also if utmp file locking is
 *	implemented in libc, syslog will use it immediately.  Thanks
 *	to Topi Miettinen <tom@medialab.sonera.net>.
 *
 * Mon Oct 12 20:49:18 MET DST 1998: Martin Schulze <joey@infodrom.north.de>
 *	Avoid logging of SIGCHLD when syslogd is in the process of
 *	exiting and closing its files.  Again thanks to Topi.
 *
 * Mon Oct 12 22:18:34 CEST 1998: Martin Schulze <joey@infodrom.north.de>
 *	Modified printline() to support 8bit characters - such as
 *	russion letters.  Thanks to Vladas Lapinskas <lapinskas@mail.iae.lt>.
 *
 * Sat Nov 14 02:29:37 CET 1998: Martin Schulze <joey@infodrom.north.de>
 *	``-m 0'' now turns of MARK logging entirely.
 *
 * Tue Jan 19 01:04:18 MET 1999: Martin Schulze <joey@infodrom.north.de>
 *	Finally fixed an error with `-a' processing, thanks to Topi
 *	Miettinen <tom@medialab.sonera.net>.
 *
 * Sun May 23 10:08:53 CEST 1999: Martin Schulze <joey@infodrom.north.de>
 *	Removed superflous call to utmpname().  The path to the utmp
 *	file is defined in the used libc and should not be hardcoded
 *	into the syslogd binary referring the system it was compiled on.
 *
 * Sun Sep 17 20:45:33 CEST 2000: Martin Schulze <joey@infodrom.ffis.de>
 *	Fixed some bugs in printline() code that did not escape
 *	control characters '\177' through '\237' and contained a
 *	single-byte buffer overflow.  Thanks to Solar Designer
 *	<solar@false.com>.
 *
 * Sun Sep 17 21:26:16 CEST 2000: Martin Schulze <joey@infodrom.ffis.de>
 *	Don't close open sockets upon reload.  Thanks to Bill
 *	Nottingham.
 *
 * Mon Sep 18 09:10:47 CEST 2000: Martin Schulze <joey@infodrom.ffis.de>
 *	Fixed bug in printchopped() that caused syslogd to emit
 *	kern.emerg messages when splitting long lines.  Thanks to
 *	Daniel Jacobowitz <dan@debian.org> for the fix.
 *
 * Mon Sep 18 15:33:26 CEST 2000: Martin Schulze <joey@infodrom.ffis.de>
 *	Removed unixm/unix domain sockets and switch to Datagram Unix
 *	Sockets.  This should remove one possibility to play DoS with
 *	syslogd.  Thanks to Olaf Kirch <okir@caldera.de> for the patch.
 *
 * Sun Mar 11 20:23:44 CET 2001: Martin Schulze <joey@infodrom.ffis.de>
 *	Don't return a closed fd if `-a' is called with a wrong path.
 *	Thanks to Bill Nottingham <notting@redhat.com> for providing
 *	a patch.
 * Thu Apr 13 05:08:10 CEST 2001: Jon Burgess <Jon_Burgess@eur.3com.com>
 *	Moved the installation of the signal handler up a little bit
 *	so it guaranteed to be available when the child is forked,
 *	hence, fixing a  race condition.  This used to create problems
 *	with UML and fast machines.
 *
 * Sat Apr 17 18:03:05 CEST 2004: Steve Grubb <linux_4ever@yahoo.com>
 *	Correct memory allocation for for commandline arguments in
 *	crunch_list().
 *
 * Thu Apr 29 12:38:39 CEST 2004: Solar Designer <solar@openwall.com>
 *	Applied Openwall paranoia patches to improve crunch_list().
 *
 * Tue May  4 16:47:30 CEST 2004: Solar Designer <solar@openwall.com>
 *	Ensure that "len" is not placed in a register, and that the
 *	endtty() signal handler is not installed too early which could
 *	cause a segmentation fault or worse.
 *
 * Tue May  4 16:52:01 CEST 2004: Solar Designer <solar@openwall.com>
 *	Adjust the size of a variable to prevent a buffer overflow
 *	should _PATH_DEV ever contain something different than "/dev/".
 *
 * Tue Nov  2 20:28:23 CET 2004: Colin Phipps <cph@cph.demon.co.uk>
 *	Don't block on the network socket, in case a packet gets lost
 *	between select and recv.
 *
 * Sun Nov  7 12:28:47 CET 2004: Martin Schulze <joey@infodrom.org>
 *	Discard any timestamp information found in received syslog
 *	messages.  This will affect local messages sent from a
 *	different timezone.
 *
 * Sun Nov  7 13:47:00 CET 2004: Martin Schulze <joey@infodrom.org>
 *	Remove trailing newline when forwarding messages.
 */


#define	MAXLINE		1024		/* maximum line length */
#define	MAXSVLINE	240		/* maximum saved line length */
#define DEFUPRI		(LOG_USER|LOG_NOTICE)
#define DEFSPRI		(LOG_KERN|LOG_CRIT)
#define TIMERINTVL	30		/* interval for checking flush, mark */

#define CONT_LINE	1		/* Allow continuation lines */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#ifdef SYSV
#include <sys/types.h>
#endif
#include <utmp.h>
#include <ctype.h>
#include <string.h>
#include <setjmp.h>
#include <stdarg.h>
#include <time.h>

#define SYSLOG_NAMES
#include <sys/syslog.h>
#include <sys/param.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/file.h>
#ifdef SYSV
#include <fcntl.h>
#else
#include <sys/msgbuf.h>
#endif
#include <sys/uio.h>
#include <sys/un.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <signal.h>

#include <netinet/in.h>
#include <netdb.h>
#include <syscall.h>
#include <arpa/nameser.h>
#include <arpa/inet.h>
#include <resolv.h>
#ifndef TESTING
#include "pidfile.h"
#endif
#include "version.h"
#include "../include/nkutil.h"//richie add for get hostname
//#include "../web_mng/cgi/cgi_util.c"
#include "../include/nkdef.h"

#if defined(__linux__)
#include <paths.h>
#endif

#ifndef UTMP_FILE
#ifdef UTMP_FILENAME
#define UTMP_FILE UTMP_FILENAME
#else
#ifdef _PATH_UTMP
#define UTMP_FILE _PATH_UTMP
#else
#define UTMP_FILE "/etc/utmp"
#endif
#endif
#endif

#ifndef _PATH_LOGCONF 
#define _PATH_LOGCONF	"/etc/syslog.conf"
#endif

#if defined(SYSLOGD_PIDNAME)
#undef _PATH_LOGPID
#if defined(FSSTND)
#define _PATH_LOGPID _PATH_VARRUN SYSLOGD_PIDNAME
#else
#define _PATH_LOGPID "/etc/" SYSLOGD_PIDNAME
#endif
#else
#ifndef _PATH_LOGPID
#if defined(FSSTND)
#define _PATH_LOGPID _PATH_VARRUN "syslogd.pid"
#else
#define _PATH_LOGPID "/etc/syslogd.pid"
#endif
#endif
#endif

#ifndef _PATH_DEV
#define _PATH_DEV	"/dev/"
#endif

#ifndef _PATH_CONSOLE
#define _PATH_CONSOLE	"/dev/console"
#endif

#ifndef _PATH_TTY
#define _PATH_TTY	"/dev/tty"
#endif

#ifndef _PATH_LOG
#define _PATH_LOG	"/dev/log"
#endif

#ifdef CONFIG_NK_CRAMFS
#define NK_PATH_LOG	"/var/run/log"
#endif

/* add to convert GMT to local time */
#ifdef NK_LOG_MSG_WITH_TIMEZONE
#include <time_zones.h>
#include <nkutil.h>
#include <nkdef.h>
#define  KdDoCmdPrint(a,b)	kd_doCommand(a, CMD_PRINT, ASH_DO_NOTHING, b)
#endif

char	*ConfFile = _PATH_LOGCONF;
char	*PidFile = _PATH_LOGPID;
char	ctty[] = _PATH_CONSOLE;

char	**parts;

static int debugging_on = 0;
static int nlogs = -1;
static int restart = 0;

#define MAXFUNIX	20

int nfunix = 1;
#ifdef CONFIG_NK_CRAMFS
char *funixn[MAXFUNIX] = { NK_PATH_LOG };
#else
char *funixn[MAXFUNIX] = { _PATH_LOG };
#endif
int funix[MAXFUNIX] = { -1, };

#ifdef UT_NAMESIZE
# define UNAMESZ	UT_NAMESIZE	/* length of a login name */
#else
# define UNAMESZ	8	/* length of a login name */
#endif
#define MAXUNAMES	20	/* maximum number of user names */
#define MAXFNAME	200	/* max file pathname length */

#define INTERNAL_NOPRI	0x10	/* the "no priority" priority */
#define TABLE_NOPRI	0	/* Value to indicate no priority in f_pmask */
#define TABLE_ALLPRI    0xFF    /* Value to indicate all priorities in f_pmask */
#define	LOG_MARK	LOG_MAKEPRI(LOG_NFACILITIES, 0)	/* mark "facility" */

/*
 * Flags to logmsg().
 */

#define IGN_CONS	0x001	/* don't print on console */
#define SYNC_FILE	0x002	/* do fsync on file after printing */
#define ADDDATE		0x004	/* add a date to the message */
#define MARK		0x008	/* this message is a mark */

/* 2007/04/04 jane: support email log */
#define MAX_F_SIZE	80		//richie: define max. file size(KB)
static int LOG_QUEUE_THR = 10;
static int LogCounter = 0;
static int MailNow = 0;
static char mail_content[MAX_F_SIZE*1000];
/* get Time Zone */
/* get file content */
static int getFileContent(char *, char *);

/*
 * This table contains plain text for h_errno errors used by the
 * net subsystem.
 */
const char *sys_h_errlist[] = {
    "No problem",						/* NETDB_SUCCESS */
    "Authoritative answer: host not found",			/* HOST_NOT_FOUND */
    "Non-authoritative answer: host not found, or serverfail",	/* TRY_AGAIN */
    "Non recoverable errors",					/* NO_RECOVERY */
    "Valid name, no data record of requested type",		/* NO_DATA */
    "no address, look for MX record"				/* NO_ADDRESS */
 };

/*
 * This structure represents the files that will have log
 * copies printed.
 */

struct filed {
#ifndef SYSV
	struct	filed *f_next;		/* next in linked list */
#endif
	short	f_type;			/* entry type, see below */
	short	f_file;			/* file descriptor */
	time_t	f_time;			/* time this was last written */
	char	*f_host;		/* host from which to recd. */
	u_char	f_pmask[LOG_NFACILITIES+1];	/* priority mask */
	union {
		char	f_uname[MAXUNAMES][UNAMESZ+1];
		struct {
			char	f_hname[MAXHOSTNAMELEN+1];
			struct addrinfo	*f_addr;
		} f_forw;		/* forwarding address */
		char	f_fname[MAXFNAME];
		char	f_mname[MAXFNAME];
	} f_un;
	char	f_prevline[MAXSVLINE];		/* last message logged */
/* add year */
#if 0
	char	f_lasttime[16];			/* time of last occurrence */
#else
	char	f_lasttime[20];			/* time of last occurrence */
#endif
	char	f_prevhost[MAXHOSTNAMELEN+1];	/* host from which recd. */
	int	f_prevpri;			/* pri of f_prevline */
	int	f_prevlen;			/* length of f_prevline */
	int	f_prevcount;			/* repetition cnt of prevline */
	int	f_repeatcount;			/* number of "repeated" msgs */
	int	f_flags;			/* store some additional flags */
};

/*
 * Intervals at which we flush out "message repeated" messages,
 * in seconds after previous message is logged.  After each flush,
 * we move to the next interval until we reach the largest.
 */
int	repeatinterval[] = { 30, 60 };	/* # of secs before flush */
#define	MAXREPEAT ((sizeof(repeatinterval) / sizeof(repeatinterval[0])) - 1)
#define	REPEATTIME(f)	((f)->f_time + repeatinterval[(f)->f_repeatcount])
#define	BACKOFF(f)	{ if (++(f)->f_repeatcount > MAXREPEAT) \
				 (f)->f_repeatcount = MAXREPEAT; \
			}
#ifdef SYSLOG_INET
#define INET_SUSPEND_TIME 180		/* equal to 3 minutes */
#define INET_RETRY_MAX 10		/* maximum of retries for getaddrinfo() */
#endif

#define LIST_DELIMITER	':'		/* delimiter between two hosts */

/* values for f_type */
#define F_UNUSED	0		/* unused entry */
#define F_FILE		1		/* regular file */
#define F_TTY		2		/* terminal */
#define F_CONSOLE	3		/* console terminal */
#define F_FORW		4		/* remote machine */
#define F_USERS		5		/* list of users */
#define F_WALL		6		/* everyone logged on */
#define F_FORW_SUSP	7		/* suspended host forwarding */
#define F_FORW_UNKN	8		/* unknown host forwarding */
#define F_PIPE		9		/* named pipe */
#define F_MAIL		10		/* mail */

char	*TypeNames[] = {
	"UNUSED",	"FILE",		"TTY",		"CONSOLE",
	"FORW",		"USERS",	"WALL",		"FORW(SUSPENDED)",
	"FORW(UNKNOWN)", "PIPE"
};

struct	filed *Files = (struct filed *) 0;
struct	filed consfile;

struct code {
	char	*c_name;
	int	c_val;
};

struct code	PriNames[] = {
	{"alert",	LOG_ALERT},
	{"crit",	LOG_CRIT},
	{"debug",	LOG_DEBUG},
	{"emerg",	LOG_EMERG},
	{"err",		LOG_ERR},
	{"error",	LOG_ERR},		/* DEPRECATED */
	{"info",	LOG_INFO},
	{"none",	INTERNAL_NOPRI},	/* INTERNAL */
	{"notice",	LOG_NOTICE},
	{"panic",	LOG_EMERG},		/* DEPRECATED */
	{"warn",	LOG_WARNING},		/* DEPRECATED */
	{"warning",	LOG_WARNING},
	{"*",		TABLE_ALLPRI},
	{NULL,		-1}
};

struct code	FacNames[] = {
	{"auth",         LOG_AUTH},
	{"authpriv",     LOG_AUTHPRIV},
	{"cron",         LOG_CRON},
	{"daemon",       LOG_DAEMON},
	{"kern",         LOG_KERN},
	{"lpr",          LOG_LPR},
	{"mail",         LOG_MAIL},
	{"mark",         LOG_MARK},		/* INTERNAL */
	{"news",         LOG_NEWS},
	{"security",     LOG_AUTH},		/* DEPRECATED */
	{"syslog",       LOG_SYSLOG},
	{"user",         LOG_USER},
	{"uucp",         LOG_UUCP},
#if defined(LOG_FTP)
	{"ftp",          LOG_FTP},
#endif
	{"local0",       LOG_LOCAL0},
	{"local1",       LOG_LOCAL1},
	{"local2",       LOG_LOCAL2},
	{"local3",       LOG_LOCAL3},
	{"local4",       LOG_LOCAL4},
	{"local5",       LOG_LOCAL5},
	{"local6",       LOG_LOCAL6},
	{"local7",       LOG_LOCAL7},
	{NULL,           -1},
};

int	Debug;			/* debug flag */
char	LocalHostName[MAXHOSTNAMELEN+1];	/* our hostname */
char	*LocalDomain;		/* our local domain name */
int	InetInuse = 0;		/* non-zero if INET sockets are being used */
int	*finet = NULL;		/* Internet datagram sockets */
int	Initialized = 0;	/* set when we have initialized ourselves */
int	MarkInterval = 20 * 60;	/* interval between marks in seconds */
#ifdef INET6
int	family = PF_UNSPEC;	/* protocol family (IPv4, IPv6 or both) */
#else
int	family = PF_INET;	/* protocol family (IPv4 only) */
#endif
int	send_to_all = 0;	/* send message to all IPv4/IPv6 addresses */
int	MarkSeq = 0;		/* mark sequence number */
int	NoFork = 0; 		/* don't fork - don't run in daemon mode */
int	AcceptRemote = 0;	/* receive messages that come via UDP */
char	**StripDomains = NULL;	/* these domains may be stripped before writing logs */
char	**LocalHosts = NULL;	/* these hosts are logged with their hostname */
int	NoHops = 1;		/* Can we bounce syslog messages through an
				   intermediate host. */

extern	int errno;

/* Function prototypes. */
int main(int argc, char **argv);
char **crunch_list(char *list);
int usage(void);
void untty(void);
void printchopped(const char *hname, char *msg, int len, int fd);
void printline(const char *hname, char *msg);
void printsys(char *msg);
void logmsg(int pri, char *msg, const char *from, int flags);
void fprintlog(register struct filed *f, char *from, int flags, char *msg);
void endtty();
void wallmsg(register struct filed *f, struct iovec *iov);
void reapchild();
const char *cvtaddr(struct sockaddr_storage *f, int len);
const char *cvthname(struct sockaddr_storage *f, int len);
void domark();
void debug_switch();
void logerror(const char *type);
void die(int sig);
#ifndef TESTING
void doexit(int sig);
#endif
void init();
void cfline(char *line, register struct filed *f);
int decode(char *name, struct code *codetab);
#if defined(__GLIBC__)
#define dprintf mydprintf
#endif /* __GLIBC__ */
static void dprintf(char *, ...);
static void allocate_log(void);
void sighup_handler();

#ifdef SYSLOG_UNIXAF
static int create_unix_socket(const char *path);
#endif
#ifdef SYSLOG_INET
static int *create_inet_sockets();
#endif
/* get file data incifer 2006/07/14 */
static int getFileContent(char *,char *);
//-->
//-->richie add for support email
int mailto(char *addr, char *line);
//<------------------------------

int main(argc, argv)
	int argc;
	char **argv;
{
	register int i;
	register char *p;
#if !defined(__GLIBC__)
	int len, num_fds;
#else /* __GLIBC__ */
#ifndef TESTING
	socklen_t len;
	ssize_t msglen;
#endif
	int num_fds;
#endif /* __GLIBC__ */
	/*
	 * It took me quite some time to figure out how this is
	 * supposed to work so I guess I should better write it down.
	 * unixm is a list of file descriptors from which one can
	 * read().  This is in contrary to readfds which is a list of
	 * file descriptors where activity is monitored by select()
	 * and from which one cannot read().  -Joey
	 *
	 * Changed: unixm is gone, since we now use datagram unix sockets.
	 * Hence we recv() from unix sockets directly (rather than
	 * first accept()ing connections on them), so there's no need
	 * for separate book-keeping.  --okir
	 */
	fd_set readfds;

#ifndef TESTING
	int	fd;
#ifdef  SYSLOG_INET
	struct sockaddr_storage frominet;
#endif
	pid_t ppid = getpid();
#endif
	int ch;
	struct hostent *hent;

	char line[MAXLINE +1];
	extern int optind;
	extern char *optarg;
	int maxfds;

#ifndef TESTING
	chdir ("/");
#endif
	for (i = 1; i < MAXFUNIX; i++) {
		funixn[i] = "";
		funix[i]  = -1;
	}

//	while ((ch = getopt(argc, argv, "a:dhf:l:m:np:rs:v")) != EOF)
	while ((ch = getopt(argc, argv, "46Aa:dhf:l:m:no:p:q:rs:v")) != EOF)
		switch((char)ch) {
		case '4':
			family = PF_INET;
			break;
#ifdef INET6
		case '6':
			family = PF_INET6;
			break;
#endif
		case 'A':
			send_to_all++;
			break;
		case 'a':
			if (nfunix < MAXFUNIX)
				funixn[nfunix++] = optarg;
			else
				fprintf(stderr, "Out of descriptors, ignoring %s\n", optarg);
			break;
		case 'd':		/* debug */
			Debug = 1;
			break;
		case 'f':		/* configuration file */
			ConfFile = optarg;
			break;
		case 'h':
			NoHops = 0;
			break;
		case 'l':
			if (LocalHosts) {
				fprintf (stderr, "Only one -l argument allowed," \
					"the first one is taken.\n");
				break;
			}
			LocalHosts = crunch_list(optarg);
			break;
		// use this to do periodical mail log
		case 'm':		/* mark interval */
			MarkInterval = atoi(optarg) * 60;
			break;
		case 'n':		/* don't fork */
			NoFork = 1;
			break;
/* 2007/04/06 jane: */
		// mail log right now (press mail button)
		case 'o':
			MailNow = atoi(optarg);
			break;
		case 'p':		/* path to regular log socket */
			funixn[0] = optarg;
			break;
/* 2007/04/06 jane: */
		// mail log queue threshold
		case 'q':
			LOG_QUEUE_THR = atoi(optarg);
			break;
		case 'r':		/* accept remote messages */
			AcceptRemote = 1;
			break;
		case 's':
			if (StripDomains) {
				fprintf (stderr, "Only one -s argument allowed," \
					"the first one is taken.\n");
				break;
			}
			StripDomains = crunch_list(optarg);
			break;
		case 'v':
			printf("syslogd %s.%s\n", VERSION, PATCHLEVEL);
			exit (0);
		case '?':
		default:
			usage();
		}
	if ((argc -= optind))
		usage();

#ifndef TESTING
	if ( !(Debug || NoFork) )
	{
		dprintf("Checking pidfile.\n");
		if (!check_pid(PidFile))
		{
			signal (SIGTERM, doexit);
			if (fork()) {
				/*
				 * Parent process
				 */
				sleep(300);
				/*
				 * Not reached unless something major went wrong.  5
				 * minutes should be a fair amount of time to wait.
				 * Please note that this procedure is important since
				 * the father must not exit before syslogd isn't
				 * initialized or the klogd won't be able to flush its
				 * logs.  -Joey
				 */
				exit(1);
			}
			num_fds = getdtablesize();
			for (i= 0; i < num_fds; i++)
				(void) close(i);
			untty();
		}
		else
		{
			fputs("syslogd: Already running.\n", stderr);
			exit(1);
		}
	}
	else
#endif
		debugging_on = 1;
#ifndef SYSV
	else
		setlinebuf(stdout);
#endif

#ifndef TESTING
	/* tuck my process id away */
	if ( !Debug )
	{
		dprintf("Writing pidfile.\n");
		if (!check_pid(PidFile))
		{
			if (!write_pid(PidFile))
			{
				dprintf("Can't write pid.\n");
				exit(1);
			}
		}
		else
		{
			dprintf("Pidfile (and pid) already exist.\n");
			exit(1);
		}
	} /* if ( !Debug ) */
#endif

	consfile.f_type = F_CONSOLE;
	(void) strcpy(consfile.f_un.f_fname, ctty);
	(void) gethostname(LocalHostName, sizeof(LocalHostName));
	if ( (p = strchr(LocalHostName, '.')) ) {
		*p++ = '\0';
		LocalDomain = p;
	}
	else
	{
		LocalDomain = "";

		/*
		 * It's not clearly defined whether gethostname()
		 * should return the simple hostname or the fqdn. A
		 * good piece of software should be aware of both and
		 * we want to distribute good software.  Joey
		 *
		 * Good software also always checks its return values...
		 * If syslogd starts up before DNS is up & /etc/hosts
		 * doesn't have LocalHostName listed, gethostbyname will
		 * return NULL. 
		 */
		hent = gethostbyname(LocalHostName);
		if ( hent )
			snprintf(LocalHostName, sizeof(LocalHostName), "%s", hent->h_name);
			
		if ( (p = strchr(LocalHostName, '.')) )
		{
			*p++ = '\0';
			LocalDomain = p;
		}
	}

	/*
	 * Convert to lower case to recognize the correct domain laterly
	 */
	for (p = (char *)LocalDomain; *p ; p++)
		if (isupper(*p))
			*p = tolower(*p);

	(void) signal(SIGTERM, die);
	(void) signal(SIGINT, Debug ? die : SIG_IGN);
	(void) signal(SIGQUIT, Debug ? die : SIG_IGN);
	(void) signal(SIGCHLD, reapchild);
	(void) signal(SIGALRM, domark);
	(void) signal(SIGUSR1, Debug ? debug_switch : SIG_IGN);
	(void) signal(SIGXFSZ, SIG_IGN);
#if 0
	(void) alarm(TIMERINTVL);
#else
	if (MailNow)
		(void) alarm(1); /* 1s for maillnow */
	else
		(void) alarm(TIMERINTVL); /* 30s periodically check-out */
#endif

	/* Create a partial message table for all file descriptors. */
	num_fds = getdtablesize();
	dprintf("Allocated parts table for %d file descriptors.\n", num_fds);
	if ( (parts = (char **) malloc(num_fds * sizeof(char *))) == \
	    (char **) 0 )
	{
		logerror("Cannot allocate memory for message parts table.");
		die(0);
	}
	for(i= 0; i < num_fds; ++i)
	    parts[i] = (char *) 0;

	dprintf("Starting.\n");
	init();
#ifndef TESTING
	if ( Debug )
	{
		dprintf("Debugging disabled, SIGUSR1 to turn on debugging.\n");
		debugging_on = 0;
	}
	/*
	 * Send a signal to the parent to it can terminate.
	 */
	if (getpid() != ppid)
		kill (ppid, SIGTERM);
#endif

	/* Main loop begins here. */
	for (;;) {
		int nfds;
		errno = 0;
		FD_ZERO(&readfds);
		maxfds = 0;
#ifdef SYSLOG_UNIXAF
#ifndef TESTING
		/*
		 * Add the Unix Domain Sockets to the list of read
		 * descriptors.
		 */
		/* Copy master connections */
		for (i = 0; i < nfunix; i++) {
			if (funix[i] != -1) {
				FD_SET(funix[i], &readfds);
				if (funix[i]>maxfds) maxfds=funix[i];
			}
		}
#endif
#endif
#ifdef SYSLOG_INET
#ifndef TESTING
		/*
		 * Add the Internet Domain Socket to the list of read
		 * descriptors.
		 */
		if ( InetInuse && AcceptRemote ) {
			for (i = 0; i < *finet; i++) {
				if (finet[i+1] != -1)
					FD_SET(finet[i+1], &readfds);
				if (finet[i+1]>maxfds) maxfds=finet[i+1];
			}
			dprintf("Listening on syslog UDP port.\n");
		}
#endif
#endif
#ifdef TESTING
		FD_SET(fileno(stdin), &readfds);
		if (fileno(stdin) > maxfds) maxfds = fileno(stdin);

		dprintf("Listening on stdin.  Press Ctrl-C to interrupt.\n");
#endif

		if ( debugging_on )
		{
			dprintf("Calling select, active file descriptors (max %d): ", maxfds);
			for (nfds= 0; nfds <= maxfds; ++nfds)
				if ( FD_ISSET(nfds, &readfds) )
					dprintf("%d ", nfds);
			dprintf("\n");
		}
		nfds = select(maxfds+1, (fd_set *) &readfds, (fd_set *) NULL,
				  (fd_set *) NULL, (struct timeval *) NULL);
		if ( restart )
		{
			dprintf("\nReceived SIGHUP, reloading syslogd.\n");
			init();
			restart = 0;
			continue;
		}
		if (nfds == 0) {
			dprintf("No select activity.\n");
			continue;
		}
		if (nfds < 0) {
			if (errno != EINTR)
				logerror("select");
			dprintf("Select interrupted.\n");
			continue;
		}

		if ( debugging_on )
		{
			dprintf("\nSuccessful select, descriptor count = %d, " \
				"Activity on: ", nfds);
			for (nfds= 0; nfds <= maxfds; ++nfds)
				if ( FD_ISSET(nfds, &readfds) )
					dprintf("%d ", nfds);
			dprintf(("\n"));
		}

#ifndef TESTING
#ifdef SYSLOG_UNIXAF
		for (i = 0; i < nfunix; i++) {
		    if ((fd = funix[i]) != -1 && FD_ISSET(fd, &readfds)) {
			memset(line, '\0', sizeof(line));
			i = recv(fd, line, MAXLINE - 2, 0);
			dprintf("Message from UNIX socket: #%d\n", fd);
			if (i > 0) {
				line[i] = line[i+1] = '\0';
				printchopped(LocalHostName, line, i + 2,  fd);
			} else if (i < 0 && errno != EINTR) {
				dprintf("UNIX socket error: %d = %s.\n", \
					errno, strerror(errno));
				logerror("recvfrom UNIX");
	      	}
				}
			}
#endif

#ifdef SYSLOG_INET
		if (InetInuse && AcceptRemote && finet) {
			for (i = 0; i < *finet; i++) {
				if (finet[i+1] != -1 && FD_ISSET(finet[i+1], &readfds)) {
					len = sizeof(frominet);
					memset(line, 0, sizeof(line));
					msglen = recvfrom(finet[i+1], line, MAXLINE - 2, 0, \
						     (struct sockaddr *) &frominet, &len);
					if (Debug) {
						const char *addr = cvtaddr(&frominet, len);
						dprintf("Message from inetd socket: #%d, host: %s\n",
							i+1, addr);
					}
					if (msglen > 0) {
						/* Note that if cvthname() returns NULL then
						   we shouldn't attempt to log the line -- jch */
						const char *from = cvthname(&frominet, len);
						if (from)
							printchopped(from, line,
								     msglen + 2,  finet[i+1]);
					} else if (msglen < 0 && errno != EINTR && errno != EAGAIN) {
						dprintf("INET socket error: %d = %s.\n", \
							errno, strerror(errno));
						logerror("recvfrom inet");
						/* should be harmless now that we set
						 * BSDCOMPAT on the socket */
						sleep(1);
					}
				}
			}
		}
#endif
#else
		if ( FD_ISSET(fileno(stdin), &readfds) ) {
			dprintf("Message from stdin.\n");
			memset(line, '\0', sizeof(line));
			line[0] = '.';
			parts[fileno(stdin)] = (char *) 0;
			i = read(fileno(stdin), line, MAXLINE);
			if (i > 0) {
				printchopped(LocalHostName, line, i+1, fileno(stdin));
		  	} else if (i < 0) {
		    		if (errno != EINTR) {
		      			logerror("stdin");
				}
		  	}
			FD_CLR(fileno(stdin), &readfds);
		  }

#endif
	}
}

int usage()
{
	fprintf(stderr, "usage: syslogd [-46Adrvh] [-l hostlist] [-m markinterval] [-n] [-p path]\n" \
		" [-s domainlist] [-f conffile]\n");
	exit(1);
}

#ifdef SYSLOG_UNIXAF
static int create_unix_socket(const char *path)
{
	struct sockaddr_un sunx;
	int fd;
	char line[MAXLINE +1];

	if (path[0] == '\0')
		return -1;

	(void) unlink(path);

	memset(&sunx, 0, sizeof(sunx));
	sunx.sun_family = AF_UNIX;
	(void) strncpy(sunx.sun_path, path, sizeof(sunx.sun_path));
	fd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (fd < 0 || bind(fd, (struct sockaddr *) &sunx,
			   sizeof(sunx.sun_family)+strlen(sunx.sun_path)) < 0 ||
	    chmod(path, 0666) < 0) {
		(void) snprintf(line, sizeof(line), "cannot create %s", path);
		logerror(line);
		dprintf("cannot create %s (%d).\n", path, errno);
		close(fd);
#ifndef SYSV
		die(0);
#endif
		return -1;
	}
	return fd;
}
#endif

#ifdef SYSLOG_INET
static int *create_inet_sockets()
{
	struct addrinfo hints, *res, *r;
	int error, maxs, *s, *socks;
	int on = 1, sockflags;

	memset(&hints, 0, sizeof(hints));
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = family;
	hints.ai_socktype = SOCK_DGRAM;
	error = getaddrinfo(NULL, "syslog", &hints, &res);
	if (error) {
		logerror("network logging disabled (syslog/udp service unknown).");
		logerror("see syslogd(8) for details of whether and how to enable it.");
		logerror(gai_strerror(error));
		return NULL;
	}

	/* Count max number of sockets we may open */
	for (maxs = 0, r = res; r; r = r->ai_next, maxs++);
	socks = malloc((maxs+1) * sizeof(int));
	if (!socks) {
		logerror("couldn't allocate memory for sockets");
		die(0);
	}

	*socks = 0;	/* num of sockets counter at start of array */
	s = socks + 1;
	for (r = res; r; r = r->ai_next) {
		*s = socket(r->ai_family, r->ai_socktype, r->ai_protocol);
		if (*s < 0) {
			logerror("socket");
			continue;
		}
		if (r->ai_family == AF_INET6) {
			if (setsockopt(*s, IPPROTO_IPV6, IPV6_V6ONLY,
				       (char *) &on, sizeof(on)) < 0) {
				logerror("setsockopt (IPV6_ONLY), suspending IPv6");
				close(*s);
				continue;
			}
		}
		if (setsockopt(*s, SOL_SOCKET, SO_REUSEADDR,
			       (char *) &on, sizeof(on)) < 0 ) {
			logerror("setsockopt(REUSEADDR), suspending inet");
			close(*s);
			continue;
		}
		/* We must not block on the network socket, in case a packet
		 * gets lost between select and recv, otherise the process
		 * will stall until the timeout, and other processes trying to
		 * log will also stall.
		 */
		if ((sockflags = fcntl(*s, F_GETFL)) != -1) {
			sockflags |= O_NONBLOCK;
			/*
			 * SETFL could fail too, so get it caught by the subsequent
			 * error check.
			 */
			sockflags = fcntl(*s, F_SETFL, sockflags);
		}
		if (sockflags == -1) {
			logerror("fcntl(O_NONBLOCK), suspending inet");
			close(*s);
			continue;
		}
		if (bind(*s, r->ai_addr, r->ai_addrlen) < 0) {
			logerror("bind, suspending inet");
			close(*s);
			continue;
		}
		(*socks)++;
		s++;
	}
	if (res)
		freeaddrinfo(res);
	if (*socks == 0) {
		logerror("no valid sockets, suspending inet");
		free(socks);
		return NULL;
	}
	return socks;
}
#endif

char **
crunch_list(list)
	char *list;
{
	int i, m, n;
	char *p, *q;
	char **result = NULL;

	p = list;
	
	/* strip off trailing delimiters */
	while (*p && p[strlen(p)-1] == LIST_DELIMITER)
		p[strlen(p)-1] = '\0';
	/* cut off leading delimiters */
	while (p[0] == LIST_DELIMITER)
		p++; 
	
	/* count delimiters to calculate the number of elements */
	for (n = i = 0; p[i]; i++)
		if (p[i] == LIST_DELIMITER) n++;
	
	if ((result = (char **)malloc(sizeof(char *) * (n + 2))) == NULL) {
		printf ("Sorry, can't get enough memory, exiting.\n");
		exit(1);
	}
	
	/*
	 * We now can assume that the first and last
	 * characters are different from any delimiters,
	 * so we don't have to care about this.
	 */
	m = 0;
	while ((q = strchr(p, LIST_DELIMITER)) && m < n) {
		result[m] = (char *) malloc((q - p + 1) * sizeof(char));
		if (result[m] == NULL) {
			printf ("Sorry, can't get enough memory, exiting.\n");
			exit(1);
		}
		memcpy(result[m], p, q - p);
		result[m][q - p] = '\0';
		p = q; p++;
		m++;
	}
	if ((result[m] = strdup(p)) == NULL) {
		printf ("Sorry, can't get enough memory, exiting.\n");
		exit(1);
	}
	result[++m] = NULL;

#if 0
	m = 0;
	while (result[m])
		dprintf ("#%d: %s\n", m, result[m++]);
#endif
	return result;
}


void untty()
#ifdef SYSV
{
	if ( !Debug ) {
		setsid();
	}
	return;
}

#else
{
	int i;

	if ( !Debug ) {
		i = open(_PATH_TTY, O_RDWR);
		if (i >= 0) {
			(void) ioctl(i, (int) TIOCNOTTY, (char *)0);
			(void) close(i);
		}
	}
}
#endif


/*
 * Parse the line to make sure that the msg is not a composite of more
 * than one message.
 */

void printchopped(hname, msg, len, fd)
	const char *hname;
	char *msg;
	int len;
	int fd;
{
	auto int ptlngth;

	auto char *start = msg,
		  *p,
	          *end,
		  tmpline[MAXLINE + 1];

	dprintf("Message length: %d, File descriptor: %d.\n", len, fd);
	tmpline[0] = '\0';
	if ( parts[fd] != (char *) 0 )
	{
		dprintf("Including part from messages.\n");
		strcpy(tmpline, parts[fd]);
		free(parts[fd]);
		parts[fd] = (char *) 0;
		if ( (strlen(msg) + strlen(tmpline)) > MAXLINE )
		{
			logerror("Cannot glue message parts together");
			printline(hname, tmpline);
			start = msg;
		}
		else
		{
			dprintf("Previous: %s\n", tmpline);
			dprintf("Next: %s\n", msg);
			strcat(tmpline, msg);	/* length checked above */
			printline(hname, tmpline);
			if ( (strlen(msg) + 1) == len )
				return;
			else
				start = strchr(msg, '\0') + 1;
		}
	}

	if ( msg[len-1] != '\0' )
	{
		msg[len] = '\0';
		for(p= msg+len-1; *p != '\0' && p > msg; )
			--p;
		if(*p == '\0') p++;
		ptlngth = strlen(p);
		if ( (parts[fd] = malloc(ptlngth + 1)) == (char *) 0 )
			logerror("Cannot allocate memory for message part.");
		else
		{
			strcpy(parts[fd], p);
			dprintf("Saving partial msg: %s\n", parts[fd]);
			memset(p, '\0', ptlngth);
		}
	}

	do {
		end = strchr(start + 1, '\0');
		printline(hname, start);
		start = end + 1;
	} while ( *start != '\0' );

	return;
}



/*
 * Take a raw input line, decode the message, and print the message
 * on the appropriate log files.
 */

void printline(hname, msg)
	const char *hname;
	char *msg;
{
	register char *p, *q;
	register unsigned char c;
	char line[MAXLINE + 1];
	int pri;

	/* test for special codes */
	pri = DEFUPRI;
	p = msg;

	if (*p == '<') {
		pri = 0;
		while (isdigit(*++p))
		{
		   pri = 10 * pri + (*p - '0');
		}
		if (*p == '>')
			++p;
	}
	if (pri &~ (LOG_FACMASK|LOG_PRIMASK))
		pri = DEFUPRI;

	memset (line, 0, sizeof(line));
	q = line;
	while ((c = *p++) && q < &line[sizeof(line) - 4]) {
		if (c == '\n')
			*q++ = ' ';
		else if (c < 040) {
			*q++ = '^';
			*q++ = c ^ 0100;
		} else if (c == 0177 || (c & 0177) < 040) {
			*q++ = '\\';
			*q++ = '0' + ((c & 0300) >> 6);
			*q++ = '0' + ((c & 0070) >> 3);
			*q++ = '0' + (c & 0007);
		} else
			*q++ = c;
	}
	*q = '\0';

	logmsg(pri, line, hname, SYNC_FILE);
	return;
}



/*
 * Take a raw input line from /dev/klog, split and format similar to syslog().
 */

void printsys(msg)
	char *msg;
{
	register char *p, *q;
	register int c;
	char line[MAXLINE + 1];
	int pri, flags;
	char *lp;

	(void) snprintf(line, sizeof(line), "vmunix: ");
	lp = line + strlen(line);
	for (p = msg; *p != '\0'; ) {
		flags = ADDDATE;
		pri = DEFSPRI;
		if (*p == '<') {
			pri = 0;
			while (isdigit(*++p))
				pri = 10 * pri + (*p - '0');
			if (*p == '>')
				++p;
		} else {
			/* kernel printf's come out on console */
			flags |= IGN_CONS;
		}
		if (pri &~ (LOG_FACMASK|LOG_PRIMASK))
			pri = DEFSPRI;
		q = lp;
		while (*p != '\0' && (c = *p++) != '\n' &&
		    q < &line[MAXLINE])
			*q++ = c;
		*q = '\0';
		logmsg(pri, line, LocalHostName, flags);
	}
	return;
}

/*
 * Decode a priority into textual information like auth.emerg.
 */
char *textpri(pri)
	int pri;
{
	static char res[20];
	CODE *c_pri, *c_fac;

	for (c_fac = facilitynames; c_fac->c_name && !(c_fac->c_val == LOG_FAC(pri)<<3); c_fac++);
	for (c_pri = prioritynames; c_pri->c_name && !(c_pri->c_val == LOG_PRI(pri)); c_pri++);

	snprintf (res, sizeof(res), "%s.%s<%d>", c_fac->c_name, c_pri->c_name, pri);

	return res;
}

time_t	now;

/*
 * Log a message to the appropriate log files, users, etc. based on
 * the priority.
 */

void logmsg(pri, msg, from, flags)
	int pri;
	char *msg;
	const char *from;
	int flags;
{
	register struct filed *f;
	int fac, prilev, lognum;
	int msglen;
	char *timestamp;

#ifdef NK_LOG_MSG_WITH_TIMEZONE
	static char buf[50];
	struct timeval	tv;
	struct tm	tm;
	time_t c_time;
	int i=0;
#endif

	dprintf("logmsg: %s, flags %x, from %s, msg %s\n", textpri(pri), flags, from, msg);

#ifndef SYSV
	omask = sigblock(sigmask(SIGHUP)|sigmask(SIGALRM));
#endif

	/*
	 * Check to see if msg looks non-standard.
	 *
	 * A message looks like
	 * Nov 17 11:42:33 CRON[
	 * 01234567890123456
	 *    ^  ^  ^  ^  ^
	 *
	 * Remote messages are not accompanied by a timestamp.
	 * Local messages are accompanied by a timestamp (program's timezone)
	 */
	msglen = strlen(msg);
	if (!(msglen < 16 || msg[3] != ' ' || msg[6] != ' ' ||
	    msg[9] != ':' || msg[12] != ':' || msg[15] != ' ')) {
		msg += 16;
		msglen -= 16;
	}

	(void) time(&now);
#ifndef NK_LOG_MSG_WITH_TIMEZONE
	timestamp = ctime(&now) + 4;
#else
	gettimeofday(&tv, NULL);

	/* Timezone */
	KdDoCmdPrint("SYSTEM TIMEZONE", buf);
	for (i=0; time_zones[i].name; i++)
	{
		if ( strlen(buf) && !strncmp(buf, time_zones[i].name, strlen(time_zones[i].name)) )
			break;
	}
	tv.tv_sec = tv.tv_sec + time_zones[i].gmt_offset * 3600;

	/* DaylightSaving */
	KdDoCmdPrint("SYSTEM DAYLIGHTSAVING", buf);
	if (!strcmp(buf, "YES"))
	{
		int	smonth,sday,emonth,eday;
		int	sDateValue=0,eDateValue=0,nowDateValue=0;  // formate=%2d%2d(month,day)
		KdDoCmdPrint("SYSTEM DAYLIGHTSTARTDATE", buf);
		sscanf(buf,"%d:%d",&smonth,&sday);
		KdDoCmdPrint("SYSTEM DAYLIGHTENDDATE", buf);
		sscanf(buf,"%d:%d",&emonth,&eday);

		memcpy(&tm, localtime(&tv.tv_sec), sizeof(struct tm));
		sDateValue=smonth*100+sday;
		eDateValue=emonth*100+eday;
		nowDateValue=(tm.tm_mon+1)*100+tm.tm_mday;
		if( ((sDateValue <= eDateValue) && ((sDateValue<=nowDateValue)&&(nowDateValue<=eDateValue))) ||
			( (sDateValue > eDateValue) && ((nowDateValue > sDateValue) ||(nowDateValue<eDateValue))) )
		{
			tv.tv_sec = tv.tv_sec + 3600;
		}
    	}

	c_time = (time_t)tv.tv_sec;
	timestamp = ctime(&c_time) + 4;
#endif /* NK_LOG_MSG_WITH_TIMEZONE */

	/* extract facility and priority level */
	if (flags & MARK)
		fac = LOG_NFACILITIES;
	else
		fac = LOG_FAC(pri);
	prilev = LOG_PRI(pri);

	/* log the message to the particular outputs */
	if (!Initialized) {
		f = &consfile;
		f->f_file = open(ctty, O_WRONLY|O_NOCTTY);

		if (f->f_file >= 0) {
			untty();
			fprintlog(f, (char *)from, flags, msg);
			(void) close(f->f_file);
			f->f_file = -1;
		}
#ifndef SYSV
		(void) sigsetmask(omask);
#endif
		return;
	}
#ifdef SYSV
	for (lognum = 0; lognum <= nlogs; lognum++) {
		f = &Files[lognum];
#else
	for (f = Files; f; f = f->f_next) {
#endif

		/* skip messages that are incorrect priority */
		if ( (f->f_pmask[fac] == TABLE_NOPRI) || \
		    ((f->f_pmask[fac] & (1<<prilev)) == 0) )
		  	continue;

		if (f->f_type == F_CONSOLE && (flags & IGN_CONS))
			continue;

		/* don't output marks to recently written files */
		if ((flags & MARK) && (now - f->f_time) < MarkInterval / 2)
			continue;

		/*
		 * suppress duplicate lines to this file
		 */
		if ((flags & MARK) == 0 && msglen == f->f_prevlen &&
		    !strcmp(msg, f->f_prevline) &&
		    !strcmp(from, f->f_prevhost)) {
/* add year */
#if 0			
			(void) strncpy(f->f_lasttime, timestamp, 15);
#else
			(void) strncpy(f->f_lasttime, timestamp, 20);
#endif
			f->f_prevcount++;
			dprintf("msg repeated %d times, %ld sec of %d.\n",
			    f->f_prevcount, now - f->f_time,
			    repeatinterval[f->f_repeatcount]);
			/*
			 * If domark would have logged this by now,
			 * flush it now (so we don't hold isolated messages),
			 * but back off so we'll flush less often
			 * in the future.
			 */
			if (now > REPEATTIME(f)) {
				fprintlog(f, (char *)from, flags, (char *)NULL);
				BACKOFF(f);
			}
		} else {
			/* new line, save it */
			if (f->f_prevcount)
				fprintlog(f, (char *)from, 0, (char *)NULL);
			f->f_prevpri = pri;
			f->f_repeatcount = 0;
/* add year */
#if 0			
			(void) strncpy(f->f_lasttime, timestamp, 15);
#else
			(void) strncpy(f->f_lasttime, timestamp, 20);
#endif
			(void) strncpy(f->f_prevhost, from,
					sizeof(f->f_prevhost));
			if (msglen < MAXSVLINE) {
				f->f_prevlen = msglen;
				(void) strcpy(f->f_prevline, msg);
				fprintlog(f, (char *)from, flags, (char *)NULL);
			} else {
				f->f_prevline[0] = 0;
				f->f_prevlen = 0;
				fprintlog(f, (char *)from, flags, msg);
			}
		}
	}
#ifndef SYSV
	(void) sigsetmask(omask);
#endif
}
#if FALSE
} /* balance parentheses for emacs */
#endif

void fprintlog(f, from, flags, msg)
	register struct filed *f;
	char *from;
	int flags;
	char *msg;
{
	struct iovec iov[6];
	register struct iovec *v = iov;
	char repbuf[80];
//===========richie add 050906: add for file rotate==================
	FILE	*real, *copy, *temp_file;
	char	*f_name;
	char	temp_f_name[30];
	long	length, length_tmp;
	long 	rotate_size=0;
	char 	line1[300];
//===================================================================
	int 	msglen; //richie add 051222: add for send email
	char 	*timestamp;
	char 	timeadd[16];
	char	complete_msg[200];
	

#ifdef SYSLOG_INET
	register int l;
	char line[MAXLINE + 1];
	time_t fwd_suspend;
	struct addrinfo hints, *ai;
	int err;
#endif

	dprintf("Called fprintlog, ");

	v->iov_base = f->f_lasttime;
/* add year */
#if 0	
	v->iov_len = 15;
#else
	v->iov_len = 20;
#endif
	v++;
	v->iov_base = " ";
	v->iov_len = 1;
	v++;
	v->iov_base = f->f_prevhost;
	v->iov_len = strlen(v->iov_base);
	v++;
	v->iov_base = " ";
	v->iov_len = 1;
	v++;
	if (msg) {
		v->iov_base = msg;
		v->iov_len = strlen(msg);
	} else if (f->f_prevcount > 1) {
		(void) snprintf(repbuf, sizeof(repbuf), "last message repeated %d times",
		    f->f_prevcount);
		v->iov_base = repbuf;
		v->iov_len = strlen(repbuf);
	} else {
		v->iov_base = f->f_prevline;
		v->iov_len = f->f_prevlen;
	}
	v++;

	dprintf("logging to %s", TypeNames[f->f_type]);

	switch (f->f_type) {
	case F_UNUSED:
		f->f_time = now;
		dprintf("\n");
		break;

#ifdef SYSLOG_INET
	case F_FORW_SUSP:
		fwd_suspend = time((time_t *) 0) - f->f_time;
		if ( fwd_suspend >= INET_SUSPEND_TIME ) {
			dprintf("\nForwarding suspension over, " \
				"retrying FORW ");
			f->f_type = F_FORW;
			goto f_forw;
		}
		else {
			dprintf(" %s\n", f->f_un.f_forw.f_hname);
			dprintf("Forwarding suspension not over, time " \
				"left: %d.\n", INET_SUSPEND_TIME - \
				fwd_suspend);
		}
		break;
		
	/*
	 * The trick is to wait some time, then retry to get the
	 * address. If that fails retry x times and then give up.
	 *
	 * You'll run into this problem mostly if the name server you
	 * need for resolving the address is on the same machine, but
	 * is started after syslogd. 
	 */
	case F_FORW_UNKN:
		dprintf(" %s\n", f->f_un.f_forw.f_hname);
		fwd_suspend = time((time_t *) 0) - f->f_time;
		if ( fwd_suspend >= INET_SUSPEND_TIME ) {
			dprintf("Forwarding suspension to unknown over, retrying\n");
			memset(&hints, 0, sizeof(hints));
			hints.ai_family = family;
			hints.ai_socktype = SOCK_DGRAM;
			if ((err = getaddrinfo(f->f_un.f_forw.f_hname, "syslog", &hints, &ai))) {
				dprintf("Failure: %s\n", gai_strerror(err));
				dprintf("Retries: %d\n", f->f_prevcount);
				if ( --f->f_prevcount < 0 ) {
					dprintf("Giving up.\n");
					f->f_type = F_UNUSED;
				}
				else
					dprintf("Left retries: %d\n", f->f_prevcount);
			}
			else {
			        dprintf("%s found, resuming.\n", f->f_un.f_forw.f_hname);
				f->f_un.f_forw.f_addr = ai;
				f->f_prevcount = 0;
				f->f_type = F_FORW;
				goto f_forw;
			}
		}
		else
			dprintf("Forwarding suspension not over, time " \
				"left: %d\n", INET_SUSPEND_TIME - fwd_suspend);
		break;

	case F_FORW:
		/* 
		 * Don't send any message to a remote host if it
		 * already comes from one. (we don't care 'bout who
		 * sent the message, we don't send it anyway)  -Joey
		 */
	f_forw:
		dprintf(" %s\n", f->f_un.f_forw.f_hname);
		if ( strcmp(from, LocalHostName) && NoHops )
			dprintf("Not sending message to remote.\n");
		else if (finet) {
			int i;
			f->f_time = now;
			(void) snprintf(line, sizeof(line), "<%d>%s", f->f_prevpri, \
				(char *) iov[4].iov_base);
			l = strlen(line);
			if (l > MAXLINE)
				l = MAXLINE;
			err = -1;
			for (ai = f->f_un.f_forw.f_addr; ai; ai = ai->ai_next) {
				for (i = 0; i < *finet; i++) {
					int lsent;
					lsent = sendto(finet[i+1], line, l, 0,
						       ai->ai_addr, ai->ai_addrlen);
					if (lsent == l) {
						err = -1;
						break;
					}
					err = errno;
				}
				if (err == -1 && !send_to_all)
					break;
			}
			if (err != -1) {
				dprintf("INET sendto error: %d = %s.\n", 
					err, strerror(err));
				f->f_type = F_FORW_SUSP;
				errno = err;
				logerror("sendto");
			}
		}
		break;
#endif

	case F_MAIL:
#if 1 /* 2007/04/04 jane: add to support log queue threshold */
		LogCounter++;
		if (LogCounter >= LOG_QUEUE_THR)
		{
			int retVal;
			if ((retVal=getFileContent(mail_content, "/var/log/logall-mail")) < 0)
				exit(0);
			else if (retVal)
			{
				system("> /var/log/logall-mail");
				LogCounter=0;
				MarkSeq = 0; // reset mail log timer
				if (mailto(f->f_un.f_mname, mail_content) < 0)
					logerror("sendto");
			}
		}
#else
                snprintf(line, sizeof(line), "%s\n", (char *) iov[4].iov_base);
		dprintf(" %s\n", f->f_un.f_mname);
	       	if (mailto(f->f_un.f_mname, line) < 0)
		       	logerror("sendto");
#endif
		break;

	case F_CONSOLE:
		f->f_time = now;
#ifdef UNIXPC
		if (1) {
#else
		if (flags & IGN_CONS) {	
#endif
			dprintf(" (ignored).\n");
			break;
		}
		/* FALLTHROUGH */

	case F_TTY:
	case F_FILE:
	case F_PIPE:

		//logmsg(LOG_SYSLOG|LOG_INFO, "do rotate");

		f->f_time = now;
		dprintf(" %s\n", f->f_un.f_fname);
		//fputs(" come here!\n", stderr);

//=========richie add 050906: add to do file switch when log file size exceed MAX_F_SIZE=====
		chmod(f->f_un.f_fname, 0666);
		real = fopen(f->f_un.f_fname, "rb");
		if(real == NULL){
			dprintf("%s file not found!!\n", f->f_un.f_fname);
			//fputs("11 file not found!!\n", stderr);
			//exit(0);
			return;
		}else{
			fseek(real, 0L, SEEK_END);
			length = ftell(real);
			dprintf("%s lenght = %d bytes\n", f->f_un.f_fname, length);
			//fputs(" come here!1\n", stderr);
		}
		if(length > MAX_F_SIZE*1000)
		{
			//logmsg(LOG_SYSLOG|LOG_INFO, "do rotate");

			rotate_size = (MAX_F_SIZE*1000)/2;
			dprintf("%s lenght = %d bytes\n", f->f_un.f_fname, length);
			strcpy(temp_f_name, f->f_un.f_fname);
			strcat(temp_f_name, "_temp");
			//fputs(" come here!2\n", stderr);

			
			dprintf("ready to copy file to %s", temp_f_name);
			copy = fopen(temp_f_name, "w+");
			if(copy == NULL)
			{
				dprintf("%s file not found!!!", temp_f_name);
				//fputs("22 file not found!!\n", stderr);
				//exit(0);
				return;
			}else{
				//fputs(" come here3!\n", stderr);
				fseek(real, rotate_size, SEEK_SET);
				length_tmp = ftell(real);
				dprintf("1.position %d\n", length_tmp);
				while(!feof(real))
				{
					length_tmp = ftell(real);
					dprintf("2.position %d\n", length_tmp);
					if(fgets(line1, 300, real) == NULL)
						dprintf("end of file\n");
					//fputs(" come here4!\n", stderr);
					break;
				}
				while(!feof(real))
				{
					fputc(fgetc(real), copy);
				}
				fclose(copy);
				fclose(real);
			}
			real = fopen(f->f_un.f_fname, "w");
			if(real == NULL){
				dprintf("%s file not found!!\n", f->f_un.f_fname);
				//fputs("33 file not found!!\n", stderr);
				//exit(0);
				return;
			}
			copy = fopen(temp_f_name, "r");
			if(copy == NULL)
			{
				dprintf("%s file not found!!!", temp_f_name);
				//fputs("44 file not found!!\n", stderr);
				//exit(0);
				return;
			}
			fseek(real, 0L, SEEK_SET);
			fseek(copy, 0L, SEEK_SET);
			while(!feof(copy))
				fputc(fgetc(copy), real);
			fclose(copy);
			fclose(real);
			unlink(temp_f_name);//remove XXX_temp
			//fputs(" come here5!\n", stderr);
		}
		else
			fclose(real);
//===================================================================
		if (f->f_type == F_TTY || f->f_type == F_CONSOLE) {
			dprintf("t4\n");
			v->iov_base = "\r\n";
			v->iov_len = 2;
		} else {
			dprintf("t5\n");
			v->iov_base = "\n";
			v->iov_len = 1;
		}
	again:
		/* f->f_file == -1 is an indicator that we couldn't
		   open the file at startup. */
		if (f->f_file == -1)
			break;

		if (writev(f->f_file, iov, 6) < 0) {
			int e = errno;

			/* If a named pipe is full, just ignore it for now
			   - mrn 24 May 96 */
			if (f->f_type == F_PIPE && e == EAGAIN)
				break;

			(void) close(f->f_file);
			/*
			 * Check for EBADF on TTY's due to vhangup() XXX
			 * Linux uses EIO instead (mrn 12 May 96)
			 */
			if ((f->f_type == F_TTY || f->f_type == F_CONSOLE)
#ifdef linux
				&& e == EIO) {
#else
				&& e == EBADF) {
#endif
				//f->f_file = open(f->f_un.f_fname, O_WRONLY|O_APPEND|O_NOCTTY);
				f->f_file = open(f->f_un.f_fname, O_WRONLY|O_APPEND|O_NOCTTY|O_CREAT);

				if (f->f_file < 0)
				{
					dprintf("t3\n");
					f->f_type = F_UNUSED;
					logerror(f->f_un.f_fname);
				} else
				{
					dprintf("t2\n");
					untty();
					goto again;
				}
			} else {
				f->f_type = F_UNUSED;
				errno = e;
				dprintf("t1\n");
				logerror(f->f_un.f_fname);
			}
		} else if (f->f_flags & SYNC_FILE)
		{
			//(void) close(temp_file);//richie
			dprintf("f->f_flags %d\n", f->f_flags);
			(void) fsync(f->f_file);
		}
		break;

	case F_USERS:
	case F_WALL:
		f->f_time = now;
		dprintf("\n");
		v->iov_base = "\r\n";
		v->iov_len = 2;
		wallmsg(f, iov);
		break;
	} /* switch */
	if (f->f_type != F_FORW_UNKN)
		f->f_prevcount = 0;
	return;		
}
#if FALSE
}} /* balance parentheses for emacs */
#endif

jmp_buf ttybuf;

void endtty()
{
	longjmp(ttybuf, 1);
}

/*
 *  WALLMSG -- Write a message to the world at large
 *
 *	Write the specified message to either the entire
 *	world, or a list of approved users.
 */

void wallmsg(f, iov)
	register struct filed *f;
	struct iovec *iov;
{
	char p[sizeof (_PATH_DEV) + UNAMESZ];
	register int i;
	int ttyf, len;
	static int reenter = 0;
	struct utmp ut;
	struct utmp *uptr;
	char greetings[200];

	(void) &len;

	if (reenter++)
		return;

	/* open the user login file */
	setutent();


	/*
	 * Might as well fork instead of using nonblocking I/O
	 * and doing notty().
	 */
	if (fork() == 0) {
		(void) signal(SIGTERM, SIG_DFL);
		(void) alarm(0);
#ifndef SYSV
		(void) signal(SIGTTOU, SIG_IGN);
		(void) sigsetmask(0);
#endif
		(void) snprintf(greetings, sizeof(greetings),
		    "\r\n\7Message from syslogd@%s at %.24s ...\r\n",
			(char *) iov[2].iov_base, ctime(&now));
		len = strlen(greetings);

		/* scan the user login file */
		while ((uptr = getutent())) {
			memcpy(&ut, uptr, sizeof(ut));
			/* is this slot used? */
			if (ut.ut_name[0] == '\0')
				continue;
			if (ut.ut_type != USER_PROCESS)
			        continue;
			if (!(strcmp (ut.ut_name,"LOGIN"))) /* paranoia */
			        continue;

			/* should we send the message to this user? */
			if (f->f_type == F_USERS) {
				for (i = 0; i < MAXUNAMES; i++) {
					if (!f->f_un.f_uname[i][0]) {
						i = MAXUNAMES;
						break;
					}
					if (strncmp(f->f_un.f_uname[i],
					    ut.ut_name, UNAMESZ) == 0)
						break;
				}
				if (i >= MAXUNAMES)
					continue;
			}

			/* compute the device name */
			strcpy(p, _PATH_DEV);
			strncat(p, ut.ut_line, UNAMESZ);

			if (f->f_type == F_WALL) {
				iov[0].iov_base = greetings;
				iov[0].iov_len = len;
				iov[1].iov_len = 0;
			}
			if (setjmp(ttybuf) == 0) {
				(void) signal(SIGALRM, endtty);
				(void) alarm(15);
				/* open the terminal */
				ttyf = open(p, O_WRONLY|O_NOCTTY);
				if (ttyf >= 0) {
					struct stat statb;

					if (fstat(ttyf, &statb) == 0 &&
					    (statb.st_mode & S_IWRITE))
						(void) writev(ttyf, iov, 6);
					close(ttyf);
					ttyf = -1;
				}
			}
			(void) alarm(0);
		}
		exit(0);
	}
	/* close the user login file */
	endutent();
	reenter = 0;
}

void sig_pipe(int signum)
{
    signum = signum;
}

int mailto(char *addr, char *line)
{
	char tmpStr[128];
	FILE *fp;
	int len; 
	FILE *mail_result;
	char res[2];
	char title[64],mailserver[64];

#if 0 /*smtpclient*/
	//--->richie add 06/01/01 for index email result
	mail_result = fopen("/etc/email_result","w+");//re-open log conf. file
	if(mail_result == NULL){
		fprintf(stderr, "can not open /etc/email_result !!\n");
	}else
	{
		sprintf(res, "1");//write 1 for success
		fwrite (res, sizeof(char), strlen(res), mail_result);
		fclose(mail_result);
	}
	//<---------------------------------------------
#endif
	signal(SIGPIPE, sig_pipe);
	kd_doCommand("LOG SMTP", CMD_PRINT, ASH_DO_NOTHING, mailserver);
	kd_doCommand("SYSTEM HOSTNAME",CMD_PRINT,ASH_DO_NOTHING,title);
	strcat(title,"_Security_Notification");
#if 0 /*smtpclient*/
	sprintf(tmpStr, "/usr/bin/smtpclient -L -s %s %s", title, addr);
#endif
	sprintf(tmpStr, "/usr/bin/msmtp --host=%s --auth=off --from %s %s",mailserver,addr,addr);
//selena(tmpStr);
	fp = popen(tmpStr, "w");
	if(!fp)
	{
		dprintf("!fp\n");
		return -1;
	}
	fprintf(fp, "To: %s\n",addr);
	fprintf(fp, "Subject: %s\n",title);
	fprintf(fp, "From: %s\n",addr);
	fprintf(fp, "User-Agent: MSMTP\n");
	len = strlen(line);
	if(fwrite(line, len, 1, fp) != len)
		if(ferror(fp))
			return -1;
	fprintf(fp, ".\n");
	pclose(fp);
	return 1;
}

void reapchild()
{
	int saved_errno = errno;
#if defined(SYSV) && !defined(linux)
	(void) signal(SIGCHLD, reapchild);	/* reset signal handler -ASP */
	wait ((int *)0);
#else
	union wait status;

	while (wait3(&status, WNOHANG, (struct rusage *) NULL) > 0)
		;
#endif
#ifdef linux
	(void) signal(SIGCHLD, reapchild);	/* reset signal handler -ASP */
#endif
	errno = saved_errno;
}

const char *cvtaddr (struct sockaddr_storage *f, int len)
{
	static char ip[NI_MAXHOST];

	if (getnameinfo((struct sockaddr *) f, len,
			ip, NI_MAXHOST, NULL, 0, NI_NUMERICHOST))
		return "???";
	return ip;
}

/*
 * Return a printable representation of a host address.
 */
const char *cvthname(struct sockaddr_storage *f, int len)
{
	static char hname[NI_MAXHOST];
	int error;
	register char *p;
	int count;

	if ((error = getnameinfo((struct sockaddr *) f, len,
				 hname, NI_MAXHOST, NULL, 0, NI_NAMEREQD))) {
		dprintf("Host name for your address (%s) unknown: %s\n", gai_strerror(error));
		if ((error = getnameinfo((struct sockaddr *) f, len,
					 hname, NI_MAXHOST, NULL, 0, NI_NUMERICHOST))) {
			dprintf("Malformed from address: %s\n", gai_strerror(error));
			return "???";
		}
		return hname;
	}
	/*
	 * Convert to lower case, just like LocalDomain above
	 */
	for (p = hname; *p ; p++)
		if (isupper(*p))
			*p = tolower(*p);

	/*
	 * Notice that the string still contains the fqdn, but your
	 * hostname and domain are separated by a '\0'.
	 */
	if ((p = strchr(hname, '.'))) {
		if (strcmp(p + 1, LocalDomain) == 0) {
			*p = '\0';
			return (hname);
		} else {
			if (StripDomains) {
				count=0;
				while (StripDomains[count]) {
					if (strcmp(p + 1, StripDomains[count]) == 0) {
						*p = '\0';
						return (hname);
					}
					count++;
				}
			}
			if (LocalHosts) {
				count=0;
				while (LocalHosts[count]) {
					if (!strcmp(hname, LocalHosts[count])) {
						*p = '\0';
						return (hname);
					}
					count++;
				}
			}
		}
	}

	return (hname);
}

void domark()
{
	register struct filed *f;
#ifdef SYSV
	int lognum;
#endif
	now = time(0);
	MarkSeq += TIMERINTVL;
	if (MarkSeq >= MarkInterval)
	{
		logmsg(LOG_INFO, "-- MARK --", LocalHostName, ADDDATE|MARK);
		MarkSeq = 0;
	}

#ifdef SYSV
	for (lognum = 0; lognum <= nlogs; lognum++)
	{
		f = &Files[lognum];
#else
	for (f = Files; f; f = f->f_next)
	{
#endif
		if (f->f_prevcount && now >= REPEATTIME(f))
		{
			dprintf("flush %s: repeated %d times, %d sec.\n",
				TypeNames[f->f_type], f->f_prevcount,
				repeatinterval[f->f_repeatcount]);
			fprintlog(f, LocalHostName, 0, (char *)NULL);
			BACKOFF(f);
		}
		/* 2007/04/06 jane: support periodical email log */
		if ( (f->f_type == F_MAIL) && ((MailNow == 1) || (MarkSeq == 0)) )
		{
			int retVal;
			if (MailNow)
			{
				MailNow = 0;
				MarkSeq = 0;	
			}
			if ((retVal=getFileContent(mail_content, "/var/log/logall-mail")) < 0)
				exit(0);
			else if (retVal)
			{
				system("> /var/log/logall-mail");
				LogCounter = 0;
				if (mailto(f->f_un.f_mname, mail_content) < 0)
					logerror("sendto");
			}
		}
	}
	(void) signal(SIGALRM, domark);
	(void) alarm(TIMERINTVL);
}

void debug_switch()

{
	dprintf("Switching debugging_on to %s\n", (debugging_on == 0) ? "true" : "false");
	debugging_on = (debugging_on == 0) ? 1 : 0;
	signal(SIGUSR1, debug_switch);
}


/*
 * Print syslogd errors some place.
 */
void logerror(const char *type)
{
	char buf[100];

	dprintf("Called logerr, msg: %s\n", type);

	if (errno == 0)
		(void) snprintf(buf, sizeof(buf), "syslogd: %s", type);
	else
		(void) snprintf(buf, sizeof(buf), "syslogd: %s: %s", type, strerror(errno));
	errno = 0;
	logmsg(LOG_SYSLOG|LOG_ERR, buf, LocalHostName, ADDDATE);
	return;
}

void die(sig)

	int sig;
	
{
	register struct filed *f;
	char buf[100];
	int lognum;
	int i;
	int was_initialized = Initialized;

	Initialized = 0;	/* Don't log SIGCHLDs in case we
				   receive one during exiting */

	for (lognum = 0; lognum <= nlogs; lognum++) {
		f = &Files[lognum];
		/* flush any pending output */
		if (f->f_prevcount)
			fprintlog(f, LocalHostName, 0, (char *)NULL);
	}

	Initialized = was_initialized;
	if (sig) {
		dprintf("syslogd: exiting on signal %d\n", sig);
		(void) snprintf(buf, sizeof(buf), "exiting on signal %d", sig);
		errno = 0;
		logmsg(LOG_SYSLOG|LOG_INFO, buf, LocalHostName, ADDDATE);
	}

	/* Close the UNIX sockets. */
        for (i = 0; i < nfunix; i++)
		if (funix[i] != -1)
			close(funix[i]);
	/* Close the inet sockets. */
	if (InetInuse && finet) {
		for (i = 0; i < *finet; i++)
			close(finet[i+1]);
		free(finet);
	}

	/* Clean-up files. */
        for (i = 0; i < nfunix; i++)
		if (funixn[i] && funix[i] != -1)
			(void)unlink(funixn[i]);
#ifndef TESTING
	(void) remove_pid(PidFile);
#endif
	exit(0);
}

/*
 * Signal handler to terminate the parent process.
 */
#ifndef TESTING
void doexit(sig)
	int sig;
{
	exit (0);
}
#endif

/*
 *  INIT -- Initialize syslogd from configuration table
 */

void init()
{
	register int i, lognum;
	register FILE *cf;
	register struct filed *f;
#ifndef TESTING
#ifndef SYSV
	register struct filed **nextp = (struct filed **) 0;
#endif
#endif
	register char *p;
	register unsigned int Forwarding = 0;
#ifdef CONT_LINE
	char cbuf[BUFSIZ];
	char *cline;
#else
	char cline[BUFSIZ];
#endif
	struct hostent *hent;

	/*
	 *  Close all open log files and free log descriptor array.
	 */
	dprintf("Called init.\n");
	Initialized = 0;
	if ( nlogs > -1 )
	{
		dprintf("Initializing log structures.\n");

		for (lognum = 0; lognum <= nlogs; lognum++ ) {
			f = &Files[lognum];

			/* flush any pending output */
			if (f->f_prevcount)
				fprintlog(f, LocalHostName, 0, (char *)NULL);

			switch (f->f_type) {
			case F_FILE:
			case F_PIPE:
			case F_TTY:
			case F_CONSOLE:
				(void) close(f->f_file);
				break;
			case F_FORW:
			case F_FORW_SUSP:
				freeaddrinfo(f->f_un.f_forw.f_addr);
				break;
			}
		}

		/*
		 * This is needed especially when HUPing syslogd as the
		 * structure would grow infinitively.  -Joey
		 */
		nlogs = -1;
		free((void *) Files);
		Files = (struct filed *) 0;
	}
	

#ifdef SYSV
	lognum = 0;
#else
	f = NULL;
#endif

	/* open the configuration file */
	if ((cf = fopen(ConfFile, "r")) == NULL) {
		dprintf("cannot open %s.\n", ConfFile);
#ifdef SYSV
		allocate_log();
		f = &Files[lognum++];
#ifndef TESTING
		cfline("*.err\t" _PATH_CONSOLE, f);
#else
		snprintf(cbuf,sizeof(cbuf), "*.*\t%s", ttyname(0));
		cfline(cbuf, f);
#endif
#else
		*nextp = (struct filed *)calloc(1, sizeof(*f));
		cfline("*.ERR\t" _PATH_CONSOLE, *nextp);
		(*nextp)->f_next = (struct filed *)calloc(1, sizeof(*f))	/* ASP */
		cfline("*.PANIC\t*", (*nextp)->f_next);
#endif
		Initialized = 1;
		return;
	}

	/*
	 *  Foreach line in the conf table, open that file.
	 */
#if CONT_LINE
	cline = cbuf;
	while (fgets(cline, sizeof(cbuf) - (cline - cbuf), cf) != NULL) {
#else
	while (fgets(cline, sizeof(cline), cf) != NULL) {
#endif
		/*
		 * check for end-of-section, comments, strip off trailing
		 * spaces and newline character.
		 */
		for (p = cline; isspace(*p); ++p);
		if (*p == '\0' || *p == '#')
			continue;
#if CONT_LINE
		strcpy(cline, p);
#endif
		for (p = strchr(cline, '\0'); isspace(*--p););
#if CONT_LINE
		if (*p == '\\') {
			if ((p - cbuf) > BUFSIZ - 30) {
				/* Oops the buffer is full - what now? */
				cline = cbuf;
			} else {
				*p = 0;
				cline = p;
				continue;
			}
		}  else
			cline = cbuf;
#endif
		*++p = '\0';
#ifndef SYSV
		f = (struct filed *)calloc(1, sizeof(*f));
		*nextp = f;
		nextp = &f->f_next;
#endif
		allocate_log();
		f = &Files[lognum++];
#if CONT_LINE
		cfline(cbuf, f);
#else
		cfline(cline, f);
#endif
		if (f->f_type == F_FORW || f->f_type == F_FORW_SUSP || f->f_type == F_FORW_UNKN) {
			Forwarding++;
		}
	}

	/* close the configuration file */
	(void) fclose(cf);

#ifdef SYSLOG_UNIXAF
	for (i = 0; i < nfunix; i++) {
		if (funix[i] != -1)
			/* Don't close the socket, preserve it instead
			close(funix[i]);
			*/
			continue;
		if ((funix[i] = create_unix_socket(funixn[i])) != -1)
			dprintf("Opened UNIX socket `%s'.\n", funixn[i]);
	}
#endif

#ifdef SYSLOG_INET
	if (Forwarding || AcceptRemote) {
		if (!finet) {
			finet = create_inet_sockets();
			if (finet) {
				InetInuse = 1;
				dprintf("Opened syslog UDP port.\n");
			}
		}
	}
	else {
		if (finet) {
			for (i = 0; i < *finet; i++)
				if (finet[i+1] != -1)
					close(finet[i+1]);
			free(finet);
			finet = NULL;
		}
		InetInuse = 0;
	}
#endif

	Initialized = 1;

	if ( Debug ) {
#ifdef SYSV
		for (lognum = 0; lognum <= nlogs; lognum++) {
			f = &Files[lognum];
			if (f->f_type != F_UNUSED) {
				printf ("%2d: ", lognum);
#else
		for (f = Files; f; f = f->f_next) {
			if (f->f_type != F_UNUSED) {
#endif
				for (i = 0; i <= LOG_NFACILITIES; i++)
					if (f->f_pmask[i] == TABLE_NOPRI)
						printf(" X ");
					else
						printf("%2X ", f->f_pmask[i]);
				printf("%s: ", TypeNames[f->f_type]);
				switch (f->f_type) {
				case F_FILE:
				case F_PIPE:
				case F_TTY:
				case F_CONSOLE:
					printf("%s", f->f_un.f_fname);
					if (f->f_file == -1)
						printf(" (unused)");
					break;

				case F_FORW:
				case F_FORW_SUSP:
				case F_FORW_UNKN:
					printf("%s", f->f_un.f_forw.f_hname);
					break;

				case F_USERS:
					for (i = 0; i < MAXUNAMES && *f->f_un.f_uname[i]; i++)
						printf("%s, ", f->f_un.f_uname[i]);
					break;
				}
				printf("\n");
			}
		}
	}

	if ( AcceptRemote )
#ifdef DEBRELEASE
		logmsg(LOG_SYSLOG|LOG_INFO, "syslogd " VERSION "." PATCHLEVEL "#" DEBRELEASE \
		       ": restart (remote reception)." , LocalHostName, \
		       	ADDDATE);
#else
		logmsg(LOG_SYSLOG|LOG_INFO, "syslogd " VERSION "." PATCHLEVEL \
		       ": restart (remote reception)." , LocalHostName, \
		       	ADDDATE);
#endif
	else
#ifdef DEBRELEASE
		logmsg(LOG_SYSLOG|LOG_INFO, "syslogd " VERSION "." PATCHLEVEL "#" DEBRELEASE \
		       ": restart." , LocalHostName, ADDDATE);
#else
		logmsg(LOG_SYSLOG|LOG_INFO, "syslogd " VERSION "." PATCHLEVEL \
		       ": restart." , LocalHostName, ADDDATE);
#endif
	(void) signal(SIGHUP, sighup_handler);
	dprintf("syslogd: restarted.\n");
}
#if FALSE
}}} /* balance parentheses for emacs */
#endif

/*
 * Crack a configuration file line
 */

void cfline(line, f)
	char *line;
	register struct filed *f;
{
	register char *p;
	register char *q;
	register int i, i2;
	char *bp;
	int pri;
	int singlpri = 0;
	int ignorepri = 0;
	int syncfile;
#ifdef SYSLOG_INET
	struct addrinfo hints, *ai;
#endif
	char buf[MAXLINE];
	char xbuf[200];

	dprintf("cfline(%s)\n", line);

	errno = 0;	/* keep strerror() stuff out of logerror messages */

	/* clear out file entry */
#ifndef SYSV
	memset((char *) f, 0, sizeof(*f));
#endif
	for (i = 0; i <= LOG_NFACILITIES; i++) {
		f->f_pmask[i] = TABLE_NOPRI;
		f->f_flags = 0;
	}

	/* scan through the list of selectors */
	for (p = line; *p && *p != '\t' && *p != ' ';) {

		/* find the end of this facility name list */
		for (q = p; *q && *q != '\t' && *q++ != '.'; )
			continue;

		/* collect priority name */
		for (bp = buf; *q && !strchr("\t ,;", *q); )
			*bp++ = *q++;
		*bp = '\0';

		/* skip cruft */
		while (strchr(",;", *q))
			q++;

		/* decode priority name */
		if ( *buf == '!' ) {
			ignorepri = 1;
			for (bp=buf; *(bp+1); bp++)
				*bp=*(bp+1);
			*bp='\0';
		}
		else {
			ignorepri = 0;
		}
		if ( *buf == '=' )
		{
			singlpri = 1;
			pri = decode(&buf[1], PriNames);
		}
		else {
		        singlpri = 0;
			pri = decode(buf, PriNames);
		}

		if (pri < 0) {
			(void) snprintf(xbuf, sizeof(xbuf), "unknown priority name \"%s\"", buf);
			logerror(xbuf);
			return;
		}

		/* scan facilities */
		while (*p && !strchr("\t .;", *p)) {
			for (bp = buf; *p && !strchr("\t ,;.", *p); )
				*bp++ = *p++;
			*bp = '\0';
			if (*buf == '*') {
				for (i = 0; i <= LOG_NFACILITIES; i++) {
					if ( pri == INTERNAL_NOPRI ) {
						if ( ignorepri )
							f->f_pmask[i] = TABLE_ALLPRI;
						else
							f->f_pmask[i] = TABLE_NOPRI;
					}
					else if ( singlpri ) {
						if ( ignorepri )
				  			f->f_pmask[i] &= ~(1<<pri);
						else
				  			f->f_pmask[i] |= (1<<pri);
					}
					else
					{
						if ( pri == TABLE_ALLPRI ) {
							if ( ignorepri )
								f->f_pmask[i] = TABLE_NOPRI;
							else
								f->f_pmask[i] = TABLE_ALLPRI;
						}
						else
						{
							if ( ignorepri )
								for (i2= 0; i2 <= pri; ++i2)
									f->f_pmask[i] &= ~(1<<i2);
							else
								for (i2= 0; i2 <= pri; ++i2)
									f->f_pmask[i] |= (1<<i2);
						}
					}
				}
			} else {
				i = decode(buf, FacNames);
				if (i < 0) {

					(void) snprintf(xbuf, sizeof(xbuf), "unknown facility name \"%s\"", buf);
					logerror(xbuf);
					return;
				}

				if ( pri == INTERNAL_NOPRI ) {
					if ( ignorepri )
						f->f_pmask[i >> 3] = TABLE_ALLPRI;
					else
						f->f_pmask[i >> 3] = TABLE_NOPRI;
				} else if ( singlpri ) {
					if ( ignorepri )
						f->f_pmask[i >> 3] &= ~(1<<pri);
					else
						f->f_pmask[i >> 3] |= (1<<pri);
				} else {
					if ( pri == TABLE_ALLPRI ) {
						if ( ignorepri )
							f->f_pmask[i >> 3] = TABLE_NOPRI;
						else
							f->f_pmask[i >> 3] = TABLE_ALLPRI;
					} else {
						if ( ignorepri )
							for (i2= 0; i2 <= pri; ++i2)
								f->f_pmask[i >> 3] &= ~(1<<i2);
						else
							for (i2= 0; i2 <= pri; ++i2)
								f->f_pmask[i >> 3] |= (1<<i2);
					}
				}
			}
			while (*p == ',' || *p == ' ')
				p++;
		}

		p = q;
	}

	/* skip to action part */
	while (*p == '\t' || *p == ' ')
		p++;

	if (*p == '-')
	{
		syncfile = 0;
		p++;
	} else
		syncfile = 1;

	dprintf("leading char in action: %c\n", *p);
	switch (*p)
	{
	case '@':
#ifdef SYSLOG_INET
		(void) strcpy(f->f_un.f_forw.f_hname, ++p);
		dprintf("forwarding host: %s\n", p);	/*ASP*/
		memset(&hints, 0, sizeof(hints));
		hints.ai_family = family;
		hints.ai_socktype = SOCK_DGRAM;
		if (getaddrinfo(p, "syslog", &hints, &ai)) {
			/*
			 * The host might be unknown due to an
			 * inaccessible nameserver (perhaps on the
			 * same host). We try to get the ip number
			 * later, like FORW_SUSP.
			 */
			f->f_type = F_FORW_UNKN;
			f->f_prevcount = INET_RETRY_MAX;
			f->f_time = time ( (time_t *)0 );
			f->f_un.f_forw.f_addr = NULL;
		} else {
			f->f_type = F_FORW;
			f->f_un.f_forw.f_addr = ai;
		}
#endif
		break;
	case '%':
		(void) strcpy(f->f_un.f_mname, ++p);
		dprintf("mail to id : %s\n", p);
	       	f->f_type = F_MAIL;
		break;

        case '|':
	case '/':
		(void) strcpy(f->f_un.f_fname, p);
		dprintf ("filename: %s\n", p);	/*ASP*/
		if (syncfile)
			f->f_flags |= SYNC_FILE;
		if ( *p == '|' ) {
			f->f_file = open(++p, O_RDWR|O_NONBLOCK);
			f->f_type = F_PIPE;
	        } else {
			f->f_file = open(p, O_WRONLY|O_APPEND|O_CREAT|O_NOCTTY,
					 0644);
			f->f_type = F_FILE;
		}
		        
	  	if ( f->f_file < 0 ){
			f->f_file = -1;
			dprintf("Error opening log file: %s\n", p);
			logerror(p);
			break;
		}
		if (isatty(f->f_file)) {
			f->f_type = F_TTY;
			untty();
		}
		if (strcmp(p, ctty) == 0)
			f->f_type = F_CONSOLE;
		break;

	case '*':
		dprintf ("write-all\n");
		f->f_type = F_WALL;
		break;

	default:
		dprintf ("users: %s\n", p);	/* ASP */
		for (i = 0; i < MAXUNAMES && *p; i++) {
			for (q = p; *q && *q != ','; )
				q++;
			(void) strncpy(f->f_un.f_uname[i], p, UNAMESZ);
			if ((q - p) > UNAMESZ)
				f->f_un.f_uname[i][UNAMESZ] = '\0';
			else
				f->f_un.f_uname[i][q - p] = '\0';
			while (*q == ',' || *q == ' ')
				q++;
			p = q;
		}
		f->f_type = F_USERS;
		break;
	}
	return;
}


/*
 *  Decode a symbolic name to a numeric value
 */

int decode(name, codetab)
	char *name;
	struct code *codetab;
{
	register struct code *c;
	register char *p;
	char buf[80];

	dprintf ("symbolic name: %s", name);
	if (isdigit(*name))
	{
		dprintf ("\n");
		return (atoi(name));
	}
	(void) strncpy(buf, name, 79);
	for (p = buf; *p; p++)
		if (isupper(*p))
			*p = tolower(*p);
	for (c = codetab; c->c_name; c++)
		if (!strcmp(buf, c->c_name))
		{
			dprintf (" ==> %d\n", c->c_val);
			return (c->c_val);
		}
	return (-1);
}

static void dprintf(char *fmt, ...)

{
	va_list ap;

	if ( !(Debug && debugging_on) )
		return;
	
	va_start(ap, fmt);
	vfprintf(stdout, fmt, ap);
	va_end(ap);

	fflush(stdout);
	return;
}


/*
 * The following function is responsible for allocating/reallocating the
 * array which holds the structures which define the logging outputs.
 */
static void allocate_log()

{
	dprintf("Called allocate_log, nlogs = %d.\n", nlogs);
	
	/*
	 * Decide whether the array needs to be initialized or needs to
	 * grow.
	 */
	if ( nlogs == -1 )
	{
		Files = (struct filed *) malloc(sizeof(struct filed));
		if ( Files == (void *) 0 )
		{
			dprintf("Cannot initialize log structure.");
			logerror("Cannot initialize log structure.");
			return;
		}
	}
	else
	{
		/* Re-allocate the array. */
		Files = (struct filed *) realloc(Files, (nlogs+2) * \
						  sizeof(struct filed));
		if ( Files == (struct filed *) 0 )
		{
			dprintf("Cannot grow log structure.");
			logerror("Cannot grow log structure.");
			return;
		}
	}
	
	/*
	 * Initialize the array element, bump the number of elements in the
	 * the array and return.
	 */
	++nlogs;
	memset(&Files[nlogs], '\0', sizeof(struct filed));
	return;
}


/*
 * The following function is resposible for handling a SIGHUP signal.  Since
 * we are now doing mallocs/free as part of init we had better not being
 * doing this during a signal handler.  Instead this function simply sets
 * a flag variable which will tell the main loop to go through a restart.
 */
void sighup_handler()

{
	restart = 1;
	signal(SIGHUP, sighup_handler);
	return;
}


static int getFileContent(char *cBuf, char *cFile)
{
	FILE *fp;
	/* purpose : 0012711  author : selena.peng date : 2010-06-24
 	* description : syslogd die (bug not big enough 256->512) */
	char tmpbuf[512];
	long curpos;
	int ret,i,j,x;
	char time[4][40],log_pri[40],log_type[40],etype[4],message[400],datatemp[256];
	char dataBuf[200*1000],buf[60],cmdBuf[100],*p,*pp;
	char dinterface[4],src[20],dst[20],prot[8],sport[8],dport[8],tmp[4][64],code[8],icmptype[4];
	char *timestamp;
	struct timeval tv;
	struct tm tm;
	time_t c_time;

	memset(cBuf, 0, sizeof(MAX_F_SIZE*1000));
		
	if ((fp = fopen(cFile,"rb")) == NULL)
	{
		return -1;
	}

	/* if file is empty */
	fseek(fp, 0L, SEEK_END);
	if ((curpos = ftell(fp)) == -1L)	
	{
		ret = -1;
		goto EXIT;
	}
	else if (curpos == 0)
	{
		ret = 0;
		goto EXIT;
	}
	fseek(fp, 0L, SEEK_SET);

#if 0
	sprintf(cBuf, "firmware version : %s%s-%s \n\n", FIRMWARE_VERSION, FIRMWARE_RC_VERSION, VENDOR_CODE);
#endif
	gettimeofday(&tv, NULL);

	/* Timezone */
	KdDoCmdPrint("SYSTEM TIMEZONE", buf);
	for (i=0; time_zones[i].name; i++)
	{
		if ( strlen(buf) && !strncmp(buf, time_zones[i].name, strlen(time_zones[i].name)) )
		{
			break;
		}
	}
	tv.tv_sec = tv.tv_sec + time_zones[i].gmt_offset * 3600;

	/* DaylightSaving */
	KdDoCmdPrint("SYSTEM DAYLIGHTSAVING", buf);
	if (!strcmp(buf, "YES"))
	{
		int	smonth,sday,emonth,eday;
		int	sDateValue=0,eDateValue=0,nowDateValue=0;  // formate=%2d%2d(month,day)
		KdDoCmdPrint("SYSTEM DAYLIGHTSTARTDATE", buf);
		sscanf(buf,"%d:%d",&smonth,&sday);
		KdDoCmdPrint("SYSTEM DAYLIGHTENDDATE", buf);
		sscanf(buf,"%d:%d",&emonth,&eday);

		memcpy(&tm, localtime(&tv.tv_sec), sizeof(struct tm));
		sDateValue=smonth*100+sday;
		eDateValue=emonth*100+eday;
		nowDateValue=(tm.tm_mon+1)*100+tm.tm_mday;
		if( ((sDateValue <= eDateValue) && ((sDateValue<=nowDateValue)&&(nowDateValue<=eDateValue))) ||
			( (sDateValue > eDateValue) && ((nowDateValue > sDateValue) ||(nowDateValue<eDateValue))) )
		{
			tv.tv_sec = tv.tv_sec + 3600;
		}
    	}

	c_time = (time_t)tv.tv_sec;
	timestamp = ctime(&c_time) + 4;
	sprintf(cBuf,"The following are logs until  %s ===\n\n",timestamp);
	while (fgets(tmpbuf, sizeof(tmpbuf), fp) != NULL)
	{
		for(j=0;tmpbuf[j]!='\0';j++)
		{			
			if(tmpbuf[j]=='\n')
			{
				tmpbuf[j]=' ';
			}
		}
		sscanf(tmpbuf,"%s %s %s %s %s", time[0],time[1],time[2], time[3],cmdBuf);
		p = strstr(tmpbuf, cmdBuf);
		/* purpose : 0012711  author : selena.peng date : 2010-06-24 */
 		/* description : syslogd die (check more) */
		if (p == NULL) 
		{
			continue;
		}
		p += strlen(cmdBuf);
		pp = strstr(p, "Log:");
		if (pp == NULL) 
		{
			p = strstr(tmpbuf, cmdBuf);
			p += strlen(cmdBuf);
			pp = strstr(p, ">");
			if (pp == NULL) 
			{
				strcpy(log_type, "Kernel");
				pp = p-1;	
			}
			else
			{
				p=pp+2;
				pp = strchr(pp, ':');
				if (pp == NULL)
				{
					continue;
				}	
				*pp = '\0';
				strcpy(log_type, p);
			}
		} 
		else
		{
			pp = strchr(pp, ':');
			if (pp == NULL)
			{
				continue;
			}
			*pp = '\0';
			strcpy(log_type, p);
		}
		strcpy(message, pp+1);

		if(strstr(message,"HTTP Basic authentication succeeded for user")!=0)		
		{
			strcpy(log_type, "Authentication Success");
		}
		else if(strstr(message,"Bad login attempt for user")!=0)		
		{
			strcpy(log_type, "Authentication Failure");		
		}

		if(!strcmp(log_type,"Connection Accepted") || (strstr(log_type,"Connection Refused - Policy violation")!=0) ||  strstr(log_type,"Blocked")!=0)
		{
			x=sscanf(message,"%*s OUT=%s  SRC=%s DST=%s %*s %*s %*s %*s %*s %*s PROTO=%s ",dinterface,src,dst,prot); 
			if(x!=6)
			{
				sscanf(message,"%*s OUT=%s  SRC=%s DST=%s %*s %*s %*s %*s %*s PROTO=%s ",dinterface,src,dst,prot);
			}
			if(!strcmp(prot,"ICMP"))
			{
				sscanf(message,"%*s OUT=%s  SRC=%s DST=%s %*s %*s %*s %*s %*s PROTO=%s TYPE=%s CODE=%s",dinterface,src,dst,prot,icmptype,code);
				sprintf(message,"%s type %s code %s  %s->%s on %s",prot,icmptype,code,src,dst,dinterface);
			}
			else			 
			{
				x=sscanf(message,"%*s OUT=%s  SRC=%s DST=%s  %*s %*s %*s %*s %*s %*s  PROTO=%s SPT=%s DPT=%s ", dinterface,src,dst,prot,sport,dport);
				if(x!=6)
				{
					sscanf(message,"%*s OUT=%s  SRC=%s DST=%s  %*s %*s %*s %*s %*s  PROTO=%s SPT=%s DPT=%s ", dinterface,src,dst,prot,sport,dport);
				}
				sprintf(message,"%s %s:%s->%s:%s on %s",prot,src,sport,dst,dport,dinterface);			 
			}
		}

		sprintf(datatemp,"%s %s %s %s %s %s\n\n",log_type,message,time[0],time[1],time[2],time[3]);
		strcat(cBuf,datatemp);
	}
	ret = 1;
EXIT:
	if (fp)
		fclose(fp);
	return ret;
}

/*
 * Local variables:
 *  c-indent-level: 8
 *  c-basic-offset: 8
 *  tab-width: 8
 * End:
 */

