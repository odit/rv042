#/bin/bash

_time_stamp=`more /etc/flash/etc/nk_sysconfig | grep CONFIG_TIME_STAMP | cut -f 2 -d "="`
_reboot_count=""
_tar_to_mirror="0"

            
if [ -f /etc/flash/mirror/mirror_config_temp.tgz ]; then
    rm -rf /etc/flash/mirror/mirror_config_temp.tgz
fi

while [ 1 ]
do

sleep 3600
#sleep 3

_time_stamp_now=`more /etc/flash/etc/nk_sysconfig | grep CONFIG_TIME_STAMP | cut -f 2 -d "="`

#echo $_time_stamp $_time_stamp_now $_reboot_count $_tar_to_mirror

if [ "$_time_stamp" != "$_time_stamp_now" ]; then
    _time_stamp="$_time_stamp_now"
    _reboot_count=""
    _tar_to_mirror="0"
else


    if [ "$_reboot_count" != "111111111111111111111111" ]; then
        _reboot_count="$_reboot_count"1
    else
    
        if [ "$_tar_to_mirror" = "0" ]; then
            cd /etc/flash/etc
            
            if [ ! -d /etc/flash/mirror ]; then
                mkdir /etc/flash/mirror
            fi
            tar -zcf /etc/flash/mirror/mirror_config_temp.tgz nk_sysconfig *.conf
            mv /etc/flash/mirror/mirror_config_temp.tgz /etc/flash/mirror/mirror_config.tgz 
            _tar_to_mirror="1"
        fi
    fi
fi 

done
