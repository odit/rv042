
#LIBS = -lnku
LIBS = ../../build/openssl-0.9.8d/libcrypto.a  ../../build/openssl-0.9.8d/libssl.a
CFLAGS += -Os -I../../build/openssl-0.9.8d/include/

nkfwmd5sum:nkfwmd5sum.o
	$(CC) $(CFLAGS) -o $@ nkfwmd5sum.o $(LIBS)

clean:
	rm -f *.o nkfwmd5sum

install:
	$(CP) nkfwmd5sum $(ROOT)/sbin/nkfwmd5sum
