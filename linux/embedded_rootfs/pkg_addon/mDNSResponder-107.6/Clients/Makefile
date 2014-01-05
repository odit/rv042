
# On OS X the dns_sd library functions are included in libSystem, which is implicitly linked with every executable
# If /usr/lib/libSystem.dylib exists, then we're on OS X, so we don't need also to link the "dns_sd" shared library

LIBS = -L../mDNSPosix/build/prod/ -ldns_sd

targets: build/dns-sd

clean:
	rm -rf build

build:
	mkdir build

build/dns-sd: build dns-sd.c
	$(CC) $(filter %.c %.o, $+) $(LIBS) -I../mDNSShared -o $@
