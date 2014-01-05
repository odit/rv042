#!/bin/bash
./configure --host=mips64-octeon-linux-gnu --prefix=/tmp/root-rootfs --sysconfdir=/etc --localstatedir=/var --libdir=`pwd`/lib-tmp --includedir=`pwd`/include-tmp --mandir=`pwd`/man-tmp --enable-shared=yes --disable-static --disable-ipv6 --enable-getifaddrs=no --with-openssl=no --with-libxml2=no --with-randomdev=no BUILD_CC=gcc CFLAGS="-s -O2"
