Summary: Network Address and Protocol Translation Daemon
Name: naptd
Version: 0.4.1
Release: 1
License: GPL
Group: System Environment/Daemons
Source: naptd-0.4.1.tar.gz

%description
NAT-PT (codename Ataga) is a loose implementation of RFC 2766 as specified by
the IETF. It runs on the GNU/Linux operating system and is designed to be easy
to setup and robust enough to make the transition to IPv6 networks a reality.
NAT-PT was designed so that it can be run on low-end, commodity hardware. It can
even work on a system with only one NIC.

%prep
%setup -q

%build
make 

%install
make install

%post
echo "Please run 'naptd-confmaker' to complete the installation process."

%files
%defattr(-,root,root)

/etc/init.d/naptd
/usr/sbin/naptd
/usr/sbin/naptd-confmaker
/usr/lib/naptd

%changelog
* Fri Jan 27 2006 Lukasz Tomicki <tomicki@o2.pl> 
- Improvements in DNS plugin.

* Tue Nov 22 2005 Lukasz Tomicki <tomicki@o2.pl> 
- Fixed routing bug when using multiple interfaces.
- Fixed bug when creating errornous PID files.

* Mon Oct 09 2005 Lukasz Tomicki <tomicki@o2.pl> 
- Fixed reentry bug on clean up code.
- Fixed service management script.
- Fixed routing problem when default route exists.
- Logging system now uses syslog.

* Wed Sep 05 2005 Lukasz Tomicki <tomicki@o2.pl> 
- First package build. Initial public release.

