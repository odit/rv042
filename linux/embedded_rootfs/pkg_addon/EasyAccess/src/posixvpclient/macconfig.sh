#!/bin/sh

chmod go+w "$1" "$1"*
chmod ugo+x "$1"uninstaller.sh "$1"dnscommand.sh
if [ -n $2 -a $2 = "true" ]
then
	sudo -K
fi
sudo -u root chown root "$1"pppwrap "$1"routewrap "$1"dnscommand "$1"route
ret_val="$?"
if [ $ret_val -eq 0 ]
then
	sudo -u root chmod ugo+xs "$1"pppwrap "$1"routewrap "$1"dnscommand "$1"route

	if [ -f "$1"ifconf ]
	then
		sudo -u root chown root "$1"ifconf
		sudo -u root chmod ugo+xs "$1"ifconf
	fi
fi
#Destroy the password token
sudo -K
exit $ret_val
