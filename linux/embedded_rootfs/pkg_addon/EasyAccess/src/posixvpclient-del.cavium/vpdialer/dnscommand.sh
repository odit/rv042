#!/bin/sh

if [ "$#" -lt 2 ]
then
	echo "Very few arguments..."
	echo "Usage : $0 [set/unset] domain nameserver1 [nameserver2] interface"
	exit 1
fi

OPERATION="$1"
DOMAIN="$2"
DNS1="$3"
DNS2="$4"
IF_NAME="$5"

#echo "Arguments : $1 $2 $3 $4"

# Service IDs of all the PPPs running in this machine
ppp_services=`echo "list State:/Network/Service/[^/]+/PPP" | /usr/sbin/scutil | /usr/bin/awk '{split($4, a, "/"); print a[4];}'`
 
for service in $ppp_services
do
        # extract the PPP interface name from the key string
		ppp_interface=`echo "show State:/Network/Service/$service/PPP" | /usr/sbin/scutil | /usr/bin/awk '/InterfaceName/ { print $3;}'`

		# set the domain and dns only for our PPP
		if [ "$ppp_interface" = "$IF_NAME" ]
        then
			if [ "$OPERATION" = "set" ]
			then
				echo "State:/Network/Service/$service/DNS"
				/usr/sbin/scutil << EOM
				get State:/Network/Service/"$service"/DNS
				d.add ServerAddresses * "$DNS1" "$DNS2"
				d.add DomainName "$DOMAIN"
				set State:/Network/Service/$service/DNS
	# there should be no white spaces before EOM ; it should be the first word in the line
EOM
			elif [ "$OPERATION" = "unset" ]
			then
				echo "remove State:/Network/Service/$service/DNS"
				echo "remove State:/Network/Service/$service/DNS" | /usr/sbin/scutil
			fi
		fi # ~if [ "$ppp_interface" = "$IF_NAME" ]
done 
