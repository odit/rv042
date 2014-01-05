#!/bin/sh

if [ $# -ne 1 ]
then
  echo "not enough variables, to be launched from pppd"
  exit
fi

for i in `/sbin/ip6tables -L CONFIGFORWARD  --line-number | grep $1 | cut -f 1 -d " "`
do
        /sbin/ip6tables -D CONFIGFORWARD $i
done

/sbin/ip6tables -F $1

/sbin/ip6tables  -X $1

exit 0
