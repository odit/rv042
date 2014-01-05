OFLAG = -o
OBJ = alg.o arp_handler.o ataga.o dns_helper.o log.o nat-mngr.o ndisc.o settings.o v4_handler.o v6_handler.o
OBJ_CONFMAKER = conf_maker.o settings.o
LIBS = -lpthread -ldl
LIBS_CONFMAKER = 
CPPFLAGS = -Wall -O3
CPP = mips64-octeon-linux-gnu-g++
CC = mips64-octeon-linux-gnu-gcc
STRIP = mips64-octeon-linux-gnu-strip
MAKE = gmake
MFLAGS = -C
PLUGINS_OFLAG = -shared -Wl,-soname,
PLUGINS_LDFLAGS = -lc -o
PLUGINS_CPPFLAGS = -fPIC

.SUFFIXES : .o .cc

.cc.o :
	$(CPP) $(CPPFLAGS) -c $<

all: main

main: ataga ataga-confmaker plugins

ataga: $(OBJ)
	$(CPP) $(LIBS) $(OFLAG) naptd $(OBJ)
	$(STRIP) naptd

ataga-confmaker: $(OBJ_CONFMAKER)
	$(CPP) $(LIBS_CONFMAKER) $(OFLAG) naptd-confmaker $(OBJ_CONFMAKER)
	$(STRIP) naptd-confmaker 

plugins: ftp_plugin dns_plugin

ftp_plugin: alg_ftp.o
dns_plugin: alg_dns.o

alg_ftp.o: alg_ftp.cc
	$(CPP) $(CPPFLAGS) $(PLUGINS_CPPFLAGS) -c $<
	$(CPP) $(PLUGINS_OFLAG)alg-ftp.so.1 $(PLUGINS_LDFLAGS) alg-ftp.so alg_ftp.o
	
alg_dns.o: alg_dns.cc
	$(CPP) $(CPPFLAGS) $(PLUGINS_CPPFLAGS) -c $<
	$(CPP) $(PLUGINS_OFLAG)alg-dns.so.1 -lresolv $(PLUGINS_LDFLAGS) alg-dns.so alg_dns.o

clean:
	rm -f *.o
	rm -f *.so
	rm -f naptd
	rm -f naptd-confmaker

install: main
	$(CP) naptd $(ROOT)/usr/sbin
	$(CP) naptd-confmaker $(ROOT)/usr/sbin
#	chown root /usr/sbin/naptd
#	chown root /usr/sbin/naptd-confmaker
#	mkdir -p /usr/lib/naptd
#	mkdir -p /usr/lib/naptd/plugins
#	cp *.so /usr/lib/naptd/plugins
#	mkdir -p /var/log/naptd
#	naptd-confmaker

conf_maker.o: conf_maker.cc
ataga.o: ataga.cc
settings.o: settings.cc
nat-mngr.o: nat-mngr.cc
log.o: log.cc
arp_handler.o: arp_handler.cc
v4_handler.o: v4_handler.cc
v6_handler.o: v6_handler.cc
alg.o: alg.cc
dns_helper.o: dns_helper.cc
ndisc.o: ndisc.cc
mysql.o: mysql.cc
