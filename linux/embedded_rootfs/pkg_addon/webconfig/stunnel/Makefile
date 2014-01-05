PROG1=	stunnel
DAEMONS= ${PROG1}
SRCS1= client.c log.c options.c protocol.c sselect.c ssl.c sthreads.c stunnel.c pty.c
LIBS += -lutil -ldl
DEFS = -DSTDC_HEADERS=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_MEMORY_H=1 -DHAVE_STRINGS_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -DHAVE_UNISTD_H=1 -DHAVE_DLFCN_H=1 -DHAVE_OPENSSL=1 -DRANDOM_FILE=\"/dev/urandom\" -DHOST=\"\" -DHAVE_LIBDL=1 -DHAVE_LIBNSL=1 -DHAVE_LIBPTHREAD=0 -DHAVE_LIBUTIL=1 -DHAVE_LIBWRAP=0 -DHAVE_DEV_PTMX=0 -DHAVE_GRP_H=1 -DHAVE_UNISTD_H=1 -DHAVE_SYS_SELECT_H=1 -DHAVE_TCPD_H=0 -DHAVE_PTHREAD_H=0 -DHAVE_SYS_IOCTL_H=1 -DHAVE_PTY_H=1 -DHAVE_STROPTS_H=1 -DHAVE_SYS_RESOURCE_H=0 -DHAVE_SNPRINTF=1 -DHAVE_VSNPRINTF=1 -DHAVE_OPENPTY=1 -DHAVE_DAEMON=1 -DHAVE_WAITPID=0 -DHAVE_WAIT4=0 -DHAVE_SYSCONF=0 -DHAVE_GETRLIMIT=1 -DHAVE_PTHREAD_SIGMASK=0 -DHAVE_SETGROUPS=1 -DHAVE_LOCALTIME_R=0 -DHAVE_CHROOT=1 -DHAVE_ENDHOSTENT=1 -DHAVE_SETSID=1
CPPFLAGS = -DLIBDIR=\"/lib\" -DCONFDIR=\"/etc/stunnel\" -DPIDFILE=\"/var/run/stunnel.pid\"

# compile flag
CFLAGS += -I../../../build/openssl-0.9.8d/include
CFLAGS += $(DEFS) $(CPPFLAGS)

#---------------
all:	${DAEMONS}
#---------------
OBJS1	+= ${SRCS1:.c=.o} ../../../build/openssl-0.9.8d/libssl.a ../../../build/openssl-0.9.8d/libcrypto.a
${PROG1}: ${OBJS1}
	${CC} $(CFLAGS) -o ${PROG1} ${LIBS} ${OBJS1} ${LDADD}
#---------------
install: ${DAEMONS}
	${CP} ${DAEMONS} $(DESTBIN)
	mkdir -p ${ROOT}/etc/stunnel
#	${CP} stunnel.pem ${ROOT}/etc/stunnel
#	${CP} stunnel-dz.pem ${ROOT}/etc/stunnel
	${CP} cacert-tm.pem ${ROOT}/etc/stunnel
#---------------
clean:
	rm -f *.o ${DAEMONS} ${DAEMONS}.elf ${DAEMONS}.gdb
#---------------
# End of Makefile
