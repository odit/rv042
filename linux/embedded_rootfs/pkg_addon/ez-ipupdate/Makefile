PROG1=	ez-ipupdate 
DAEMONS= ${PROG1} 
SRCS1= ez-ipupdate.c conf_file.c md5.c cache_file.c pid_file.c 
VERSION= 3.0.11b7

LDADD += -lnku

#---------------
all:	${DAEMONS}
#---------------
OBJS1	+= ${SRCS1:.c=.o}
${PROG1}: ${OBJS1}
	${CC} $(CFLAGS) -o ${PROG1} ${LIBS} ${OBJS1} ${LDADD}
#---------------
install: ${DAEMONS}
	${CP} ${DAEMONS} $(DESTBIN)
#---------------
clean:
	rm -f *.o ${DAEMONS} ${DAEMONS}.elf ${DAEMONS}.gdb
#---------------
# End of Makefile


