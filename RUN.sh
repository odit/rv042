#!/bin/bash

cp -f ./environment/typesizes.h ./tools/mips64-octeon-linux-gnu/sys-root/usr/include/bits/typesizes.h 
cp -f ./environment/fs.h ./tools/mips64-octeon-linux-gnu/sys-root/usr/include/linux/fs.h
cp -f ./environment/limits.h ./tools/mips64-octeon-linux-gnu/sys-root/usr/include/linux/limits.h
cp -f ./environment/posix_types.h ./tools/mips64-octeon-linux-gnu/sys-root/usr/include/linux/posix_types.h

cp -f ./environment/nkuserlandconf.h ./tools/mips64-octeon-linux-gnu/sys-root/usr/include/nkuserlandconf.h
cp -f ./environment/nkdef.h ./tools/mips64-octeon-linux-gnu/sys-root/usr/include/nkdef.h
cp -f ./environment/nkutil.h ./tools/mips64-octeon-linux-gnu/sys-root/usr/include/nkutil.h
cp -f ./environment/sysconfig.h ./tools/mips64-octeon-linux-gnu/sys-root/usr/include/sysconfig.h
cp -f ./environment/rtnetlink.h ./tools/mips64-octeon-linux-gnu/sys-root/usr/include/linux/rtnetlink.h
cp -f ./environment/time_zones.h ./tools/mips64-octeon-linux-gnu/sys-root/usr/include/time_zones.h

source env-setup OCTEON_CN50XX

cd linux/embedded_rootfs/pkg_addon/iptables-1.3.7
make clean; make clean; make clean; make clean
cd -

export PATH=$PATH:$PWD/tools/bin

cd linux
make kernel
# fix permission
chown -R lenny:lenny ./
