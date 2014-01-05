#!/bin/sh
chown root:root "$1"pppd.vp "$1"route "$1"ip-up "$1"ip-down "$1"pppwrap "$1"routewrap "$1"dnscommand
chmod ugo+xs "$1"pppd.vp "$1"route "$1"ip-up "$1"ip-down "$1"pppwrap "$1"routewrap "$1"dnscommand

if [ -f "$1"ifconf ]
then
	chown root:root "$1"ifconf
	chmod ugo+xs "$1"ifconf
fi

if [ $(/sbin/lsmod | grep -c "n_hdlc") == 0 ]
then
	/sbin/modprobe -q n_hdlc
fi

exit
