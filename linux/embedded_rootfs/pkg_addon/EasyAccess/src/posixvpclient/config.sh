#!/bin/sh
chown root:root /usr/VPClient/pppd.vp /usr/VPClient/route /usr/VPClient/ip-up /usr/VPClient/ip-down
chmod ugo+xs /usr/VPClient/pppd.vp /usr/VPClient/route /usr/VPClient/ip-up /usr/VPClient/ip-down

if [ $(lsmod | grep -c "n_hdlc") == 0 ]
then
	insmod n_hdlc
fi
