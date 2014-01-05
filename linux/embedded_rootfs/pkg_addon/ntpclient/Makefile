PROG=	ntpclient
SRCS=	ntpclient.c phaselock.c

LDADD += -lnku
#---------------
OBJS	+= ${SRCS:.c=.o}
#---------------
all:	${PROG}
#---------------
${PROG}: ${OBJS}
	${CC} $(CFLAGS) -o ${PROG} ${LIBS} ${OBJS} ${LDADD}
#---------------
clean:
	rm -f *.o ${PROG} ${PROG}.elf ${PROG}.gdb
#---------------
install: ${PROG}
	${CP} ${PROG} $(DESTBIN)
#---------------
# End of Makefile
