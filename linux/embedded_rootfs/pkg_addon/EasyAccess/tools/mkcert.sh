#!/bin/sh

#Create a RSA Cert.
openssl req -new -nodes -newkey rsa:1024 -x509 -keyout .server.key -out .server.crt -days 700

#Moving the Cert to new Location:
mv .server.crt ../var/cert/default/server.crt
mv .server.key ../var/cert/default/server.key
touch ../var/cert/password.sh
chmod +x ../var/cert/password.sh
