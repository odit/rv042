
all: cramfs
#	gcc -s -Wall -o buildimage buildimage.c
#	gcc -s -Wall -o buildboot buildboot.c
	#gcc -s -Wall -o buildmac buildmac.c
#	gcc -s -Wall -o buildinfo buildinfo.c
	#gcc -s -Wall -o buildflash buildflash.c
#	gcc -s -Wall -o buildflashfull buildflashfull.c
#	gcc -s -Wall -o addchecksum addchecksum.c
	#gcc -s -Wall -o showchecksum showchecksum.c

	./buildinfo

cramfs:
	make -C cramfs-1.1 -f Makefile

install:

clean:
	rm -f buildimage
	rm -f buildboot
	rm -f buildflash
	rm -f buildflashfull
	rm -f addchecksum
	rm -f showchecksum
	rm -f buildinfo
	make -C cramfs-1.1 clean
