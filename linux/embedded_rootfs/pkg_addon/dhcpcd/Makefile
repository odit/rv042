###############################################################################
#									      #
# 				Dhcpcd Makefile				      #
#									      #
# 	       Copyright (C) Sergei A. Viznyuk, January, 1998                 #
#			      All rights reserved.			      #
#									      #
###############################################################################

###########################
# PARAMETERS TO PLAY WITH #
###########################

#ADDS= -m586
#DEFS+= -DDEBUG		# for heavy debugging
#DEFS+= -DARPCHECK

###########################
# NOTHING BELOW NEEDS YOU #
###########################

# ifeq ($(origin OSNAME), undefined)
# OSNAME=$(shell uname)
# endif
# ifeq ($(origin ARCH), undefined)
# ARCH=$(shell uname -m)
# endif
# 
# ifeq ($(OSNAME),Linux)
# LINUX_VERSION=$(shell uname -r | cut -d\. -f1,2)
# ifeq ($(LINUX_VERSION),2.0)
# ADDS+= -DOLD_LINUX_VERSION
# endif
# CFLAGS = -O2 $(ADDS) -fomit-frame-pointer -Wall $(DEFS)
# ifeq ($(ARCH),i386)
# CFLAGS+=
# endif
# ifeq ($(ARCH),i486)
# CFLAGS+= -m486 -malign-loops=2 -malign-jumps=2 -malign-functions=2
# endif
# ifeq ($(ARCH),i586)
# CFLAGS+=
# endif
# ifeq ($(ARCH),i686)
# CFLAGS+=
# endif
# LDFLAGS = -s -O2 -static
# INCLUDES = -I.
# else
# CFLAGS = -O2
# LDFLAGS =
# INCLUDES = -I.
# endif

PROG1=	dhcpcd 
PROG2=	icmprequest 

DAEMONS= ${PROG1} #${PROG2} 

OBJS1= peekfd.o udpipgen.o buildmsg.o signals.o arp.o client.o dhcpcd.o
OBJS2= icmprequest.o icmpipgen.o udpipgen.o peekfd.o

LDADD += -lnku
#---------------
all:	${DAEMONS}
#---------------
${PROG1}: ${OBJS1}
	${CC} $(CFLAGS) -o ${PROG1} ${LIBS} ${OBJS1} ${LDADD}
#---------------
${PROG2}: ${OBJS2}
	$(CC) -s -O2 -o $@ $^
	@(ls -l $@)
#---------------
udpipgen.o:	udpipgen.c udpipgen.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $<
#---------------
icmpipgen.o:	icmpipgen.c icmpipgen.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $<
#---------------
buildmsg.o:	buildmsg.c client.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $<
#---------------
signals.o:	signals.c dhcpcd.h client.h pathnames.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $<
#---------------
arp.o:	arp.c client.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $<
#---------------
client.o:	client.c client.h buildmsg.h udpipgen.h pathnames.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $<
#---------------
dhcpcd.o:	dhcpcd.c dhcpcd.h signals.h client.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $<
#---------------
icmprequest.o:	icmprequest.c icmpipgen.c icmpipgen.h udpipgen.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $<
#---------------
install: ${DAEMONS}
	${CP} ${DAEMONS} $(DESTSBIN)
#---------------
clean:
	rm -f *.o ${DAEMONS} ${DAEMONS}.elf ${DAEMONS}.gdb
#---------------
# End of Makefile
