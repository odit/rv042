#!/bin/sh

if [ $# -ne 1 ]
then
  echo "not enough variables, to be launched from pppd"
  exit
fi

for i in `/sbin/iptables -L CONFIGFORWARD  --line-number | grep $1 | cut -f 1 -d " "`
do
        /sbin/iptables -D CONFIGFORWARD $i
done

/sbin/iptables -F $1

/sbin/iptables  -X $1

exit 0
