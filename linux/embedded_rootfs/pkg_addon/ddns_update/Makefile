#LIBS = -lnku
PROG1=	ddns_update
DAEMONS= ${PROG1} 
SRCS1= ddns_update.c base64.c md5.c
VERSION= 3.0.11b7
LDADD += -lnku
#---------------
all:
#all:	${DAEMONS}
#---------------
OBJS1	+= ${SRCS1:.c=.o}
${PROG1}: ${OBJS1}
	${CC} $(CFLAGS) -o ${PROG1} ${LIBS} ${OBJS1} ${LDADD}
#---------------
install:
	${CP} ${DAEMONS} $(DESTBIN)
#---------------
clean:
	rm -f *.o ${DAEMONS} ${DAEMONS}.elf ${DAEMONS}.gdb
#---------------
# End of Makefile
