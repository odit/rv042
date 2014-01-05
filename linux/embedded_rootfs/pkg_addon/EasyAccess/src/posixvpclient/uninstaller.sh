#!/bin/sh
sudo -K
sudo -u root rm -rf $1
ret_val="$?"
sudo -K
exit $ret_val
