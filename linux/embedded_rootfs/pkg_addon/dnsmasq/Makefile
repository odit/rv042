PROG1=	dnsmasq 
DAEMONS= ${PROG1} 
LIBS= -lnku
SRCS1=	cache.c rfc1035.c dhcp.c util.c option.c forward.c network.c dnsmasq.c

#---------------
all:	${DAEMONS}
#---------------
OBJS1	+= ${SRCS1:.c=.o}

${PROG1}: ${OBJS1}
	${CC} $(CFLAGS) -o ${PROG1} ${OBJS1} ${LDADD} ${LIBS}
#---------------
clean:
	rm -f *.o ${DAEMONS} ${DAEMONS}.elf ${DAEMONS}.gdb
#---------------
# End of Makefile
install: ${DAEMONS}
	${CP} ${DAEMONS} $(DESTSBIN)
	${CP} godnsmasq $(DESTETC)
#---------------
# End of Makefile

