#/bin/bash

while [ 1 ]
do
sleep 3600
cd /var/log
#purpose:0013264, author:selena, description:new log backup path
tar -zcf /etc/flash/log.tgz ./*
done
