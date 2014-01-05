#!/bin/sh

_file_dir=$1
if [ ! -d $_file_dir ]; then
    mkdir $_file_dir
fi
cd $_file_dir

if [ "$#" == "1" ]; then
    cp -rf $_file_dir"config.zip" /tmp/config.zip.save_for_second
    /usr/bin/openssl enc -d -des3 -in $_file_dir"config.zip" -out /tmp/config.zip.decode -k `more /tmp/splitDB/SYSTEM | grep ^PASSWD= | cut -f 2 -d "="`

    if [ -z `more /tmp/config.zip.decode | grep ^PASSWD=` ]; then
        exit 0
    fi

    rm -rf /tmp/config.zip.save_for_second
    mv /tmp/config.zip.decode $_file_dir"config.zip"
fi

if [ "$#" == "2" ]; then
    /usr/bin/openssl enc -d -des3 -in /tmp/config.zip.save_for_second -out /tmp/config.zip.input -k $2

    rm -rf /tmp/config.zip.save_for_second
    if [ -z `more /tmp/config.zip.input | grep ^PASSWD=` ]; then
        exit 0
    fi
    mv /tmp/config.zip.input $_file_dir"config.zip"
fi

sed /"==>md5sum"/d config.zip > config.zip.temp;
echo "==>md5sum 0" >> config.zip.temp

if [ `tail -1 config.zip | cut -f 2 -d " "` != `md5sum config.zip.temp | cut -f 1 -d " "` ]; then
    rm -rf config.zip
fi
    rm -rf config.zip.temp
