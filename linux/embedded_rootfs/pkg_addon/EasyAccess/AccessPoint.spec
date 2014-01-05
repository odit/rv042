Summary: Cavium Networks EasyAccess SSL VPN Server.
Name: AccessPoint
Version: 1 2 
Release: 9
Copyright: Cavium Networks
Group: System Environment/Base
BuildRoot: /tmp/build/

%description
 SSL VPN Solution for Redhat 8.0

%prep

%build

%install

%clean

%files
/usr/local/src/EasyAccess/

%post
chmod -R 777 /usr/local/src/EasyAccess/var/msg
chmod -R 777 /usr/local/src/EasyAccess/var/conf
install -d /usr/local/src/EasyAccess/var/cert/default
echo "Please Create a new Certificate using mkcert.sh in
      /usr/local/src/EasyAccess/tools/mkcert.sh"
