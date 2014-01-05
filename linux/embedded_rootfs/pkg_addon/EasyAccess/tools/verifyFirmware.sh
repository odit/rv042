#!/bin/sh

IMAGE_PATH=/var/images
IMAGE_NAME=$IMAGE_PATH/EasyAccess.tgz
POST_INSTALL=/bin/postInstall.sh
ERR_OUT=$IMAGE_PATH/firmware.err

function clean_exit()
{
   rm -rf /tmp/sig.chk
   rm -rf /tmp/build.chk
   exit 
}

if [ -e $IMAGE_NAME ]
then
	#echo "starting Verification." 

	rm -rf $ERR_OUT
	tar -tzf $IMAGE_NAME | grep -v build.chk  > /tmp/sig.chk
	if [ "$?" -ne "0" ]
	then
	   echo "Improper image archive" >> $ERR_OUT
           clean_exit
	fi

        cd /tmp; tar -xvz build.chk -f $IMAGE_NAME

        diff=`diff /tmp/build.chk  /tmp/sig.chk`
        if [ ! -z "$diff" ]
        then
              echo "Uploaded Image is corrupt" >> $ERR_OUT
              echo $diff >> $ERR_OUT
              clean_exit
        fi
	
	echo "Upload successful. Reboot the device for changes to take effect." >> $ERR_OUT
else
        if [ ! -e "$ERR_OUT" ]
        then
             echo "" > $ERR_OUT
        fi
fi

clean_exit
