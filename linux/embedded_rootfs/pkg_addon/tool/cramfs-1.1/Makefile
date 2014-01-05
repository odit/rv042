CC = gcc
CFLAGS = -W -Wall -O2 -g
CPPFLAGS = -I.
LDLIBS = -lz
PROGS = mkcramfs cramfsck

#all: $(PROGS)
all:
	$(CC) $(CFLAGS) $(CPPFLAGS) mkcramfs.c $(LDLIBS) -o mkcramfs
	$(CC) $(CFLAGS) $(CPPFLAGS) cramfsck.c $(LDLIBS) -o cramfsck

distclean clean:
	rm -f $(PROGS)

.PHONY: all clean
