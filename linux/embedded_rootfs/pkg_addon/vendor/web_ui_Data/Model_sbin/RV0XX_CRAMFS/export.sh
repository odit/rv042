#!/bin/sh

_file_dir=$1
_make_path="/tmp/config_exp/"
_download_file="config.zip"

#if [ -d $_make_path ]; then
#    cd $_make_path
#    rm -rf ./*
#else
#    mkdir $_make_path
#fi

rm -rf $_make_path
mkdir $_make_path

cp -rf "$_file_dir"*.conf $_make_path
cp -rf "$_file_dir"sysconfig $_make_path

for i in `ls $_make_path` 
do
   echo "==>FILENAME $i" >> $_make_path$_download_file
   cat $_make_path$_download_file $_make_path$i > "$_make_path"tmpfile
   cp -rf "$_make_path"tmpfile $_make_path$_download_file
done

echo "==>md5sum 0" >> $_make_path$_download_file
echo "==>md5sum "`md5sum /tmp/config_exp/$_download_file | cut -f 1 -d " "` >> "$_make_path"tmpfile

openssl enc -e -des3 -in "$_make_path"tmpfile -out /tmp/$_download_file -k `more "$_make_path"tmpfile | grep ^PASSWD= | cut -f 2 -d "="`

rm -rf $_make_path

