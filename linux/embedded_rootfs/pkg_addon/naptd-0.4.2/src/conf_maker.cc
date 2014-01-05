/***************************************************************************
 *  conf_maker.cc : This file is part of 'ataga'
 *  created on: Thu Jun  9 10:35:20 CDT 2005
 *
 *  (c) 2005 by Lukasz Tomicki <lucas.tomicki@gmail.com>
 * 
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <iostream>
#include <string>
#include <vector>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include "settings.h"
#include "enum-settings.h"

using namespace std;

char config_file[1024];

void print_usage(void);
void clean_up(int);
void get_all(istream&, char);

CSettings *pSettings;
	
int main(int argc, char** argv)
{
	strcpy(config_file, "/etc/naptd.conf");	
	
	char o;
	while ((o = getopt(argc, argv, "hc:")) != -1) {
		switch (o) {
			case 'h':
				print_usage();
				exit(0);
			break;
			
			case 'c':
				strncpy(config_file, optarg, 1024);
			break;
			
			default:
				continue;
		}
	}
	
	pSettings = new CSettings(config_file);

	cout << "Ataga IPv4/IPv6 NAPT Configuration Maker" << endl;
	cout << "(c) 2005 by Lukasz Tomicki <lucas.tomicki@gmail.com>" << endl << endl;
	
	cout << "Do you want to create a new configuration? [Y/n]" << endl;
	
	char decision = cin.get();
	get_all(cin, decision);
	
	if (decision == 'n')
		clean_up(0);
	
	
	pSettings->DropData();
	pSettings->SetPointerData(argv[0], strlen(argv[0]));
	
	int number_of_blocks(0);
	bool more(false);
	
	cout << "Do you want IPv4 addresses from the outside interfaces to be automatically used as part of the NAT pool? [Y/n]" << endl;
	
	decision = cin.get();
	//cin.get();
	get_all(cin, decision);
	
	bool get_addresses(true);
	
	if (decision == 'n') {
		get_addresses = false;
	}
	pSettings->Set(get_addresses, get_v4_from_outside_int);
	
	if (get_addresses) {
		cout << "Do you want to configure additional address as part of your NAT pool? [y/N]" << endl;
		decision = cin.get();
		get_all(cin, decision);
	
		if (decision == 'y')
			get_addresses = false;
	}
	
	string start_ip, end_ip;
	
	if (!get_addresses) {
		string start_port, end_port;
		
		cout << "You need to create a public IPv4 address pool. Enter the pool\'s starting IP." << endl;
		
		do {
			more = false;
			
			cout << "starting IP: ";
			
			cin >> start_ip;
			cin.get();
			
			cout << "ending IP (inclusive) [" << start_ip << "]: ";
			
			end_ip = start_ip;
			
			int c = cin.peek();
			
			if ((char)c != '\n') {
				cin >> end_ip;
			}
			
			cin.get();
			
			pSettings->SetPointerData(start_ip.c_str(), start_ip.size());
			pSettings->SetPointerData(end_ip.c_str(), end_ip.size());
			++number_of_blocks;
			
			cout << endl << "Enter the port range to use for this address pool." << endl;
			cout << "starting port [1050]: ";
			
			int start_port(1050), end_port(65000);
			
			c = cin.peek();
			
			if ((char)c != '\n') {
				cin >> start_port;
			}
			
			cin.get();
			
			cout << "ending port (inclusive) [65000]: ";
			
			c = cin.peek();
			
			if ((char)c != '\n') {
				cin >> end_port;
			}
			
			cin.get();
			
			pSettings->SetPointerData(&start_port, sizeof(int));
			pSettings->SetPointerData(&end_port, sizeof(int));
			
			cout << endl << "Do you want to enter more address pools? [y/N]" << endl;
			
			decision = cin.get();
			
			if (decision == 'y')
				more = true;
				
			get_all(cin, decision);
		} while(more);
	}
	pSettings->Set(number_of_blocks, number_of_public_ip_blocks);
	
	cout << "Do you want to create a pool of public IPv4 addresses that will allow incoming connections to be dynamically mapped to appropriate IPv6 addresses? [y/N]" << endl;
	
	number_of_blocks = 0;
	decision = cin.get();
	get_all(cin, decision);
	
	if (decision == 'y') {
		cout << "Enter the first public IPv4 address pool to use." << endl;
		
		do {
			more = false;
			
			cout << "starting IP: ";
			
			cin >> start_ip;
			cin.get();
			
			cout << "ending IP (inclusive) [" << start_ip << "]: ";
			
			end_ip = start_ip;
			
			int c = cin.peek();
			
			if ((char)c != '\n') {
				cin >> end_ip;
			}
			
			cin.get();
			
			pSettings->SetPointerData(start_ip.c_str(), start_ip.size());
			pSettings->SetPointerData(end_ip.c_str(), end_ip.size());
			++number_of_blocks;
			
			cout << endl << "Do you want to enter more address pools? [y/N]" << endl;
			
			decision = cin.get();
			
			if (decision == 'y')
				more = true;
			
			get_all(cin, decision);
		} while(more);
	}
	pSettings->Set(number_of_blocks, number_of_priv_blocks);
	
	cout << "Do you want to create static mappings of public IPv4 addresses that will allow incoming connections to reach IPv6 hosts? [y/N]" << endl;
	
	number_of_blocks = 0;
	decision = cin.get();
	get_all(cin, decision);
	
	if (decision == 'y') {
		do {
			more = false;
			cout << "IPv4 address: ";
			
			cin >> start_ip;
			
			cout << "IPv6 address: ";
			
			cin >> end_ip;
			
			pSettings->SetPointerData(start_ip.c_str(), start_ip.size());
			pSettings->SetPointerData(end_ip.c_str(), end_ip.size());
			++number_of_blocks;
			
			decision = cin.get();
			cout << endl << "Do you want to enter another static mapping? [y/N]" << endl;
			
			decision = cin.get();
			
			if (decision == 'y')
				more = true;
			
			get_all(cin, decision);
		} while (more);
	}
	pSettings->Set(number_of_blocks, number_of_static_mappings);
	
	string int_name;
	int number_of_interfaces(0);
	
	struct if_nameindex *listsave;
	struct if_nameindex *iflist;
	
	listsave = if_nameindex();
	iflist = listsave;
	
	vector<string> interfaces;
		
	for (; iflist->if_name != 0; ++iflist) {
		if (!strcmp(iflist->if_name, "lo"))
			continue;
		
		string interface(iflist->if_name);
		interfaces.push_back(interface);
	}
		
	if_freenameindex(listsave);
	
	cout << "Enter the name of the first inside (IPv6) interface that you want NAT-PT to listen on." << endl;
	
	do {
		more = false;
		cout << "interface ";
		
		if (interfaces.size()) {
			cout << "(";
			
			vector<string>::iterator i = interfaces.begin();
			
			while (i != interfaces.end()) {
				cout << i->c_str();
				++i;
				if (i != interfaces.end())
					cout << " ";
			}
			cout << ")";
		}	
		cout << ": ";
		
		cin >> int_name;
		pSettings->SetPointerData(int_name.c_str(), int_name.size());
		++number_of_interfaces;
		decision = cin.get();
		
		/*
		if (interfaces.size()) {
			vector<string>::iterator i = interfaces.begin();
			
			while (i != interfaces.end()) {
				if (*i == int_name) {
					interfaces.erase(i);
					break;
				}
				++i;
			}
		}	
		*/
		
		if (interfaces.size() > 1) {
			cout << endl << "Do you want to enter more interfaces? [y/N]" << endl;
			
			decision = cin.get();
			
			if (decision == 'y')
				more = true;
			
			get_all(cin, decision);
		}
		
	} while (more);
	
	pSettings->Set(number_of_interfaces, number_of_inside_interfaces);
	
	number_of_interfaces = 0;
	
	cout << "Enter the name of the first outside (IPv4) interface that you want NAT-PT to listen on." << endl;
	
	do {
		more = false;
		cout << "interface ";
		
		if (interfaces.size()) {
			cout << "(";
			
			vector<string>::iterator i = interfaces.begin();
			
			while (i != interfaces.end()) {
				cout << i->c_str();
				++i;
				if (i != interfaces.end())
					cout << " ";
			}
			cout << ")";
		}	
		cout << ": ";
		
		cin >> int_name;
		pSettings->SetPointerData(int_name.c_str(), int_name.size());
		++number_of_interfaces;
		decision = cin.get();
		
		/* seconds
		if (interfaces.size()) {
			vector<string>::iterator i = interfaces.begin();
			
			while (i != interfaces.end()) {
				if (*i == int_name) {
					interfaces.erase(i);
					break;
				}
				++i;
			}
		}	
		*/
		
		if (interfaces.size()) {
			cout << endl << "Do you want to enter more interfaces? [y/N]" << endl;
		
			decision = cin.get();
			
			if (decision == 'y')
				more = true;
			
			get_all(cin, decision);
		}
	} while (more);
	
	pSettings->Set(number_of_interfaces, number_of_outside_interfaces);
	
	int tcp_timeout(86400), udp_timeout(3600), icmp_timeout(30);
	cout << "Enter the TCP translation timeout in seconds [" << tcp_timeout << "]: ";
	
	int c = cin.peek();
			
	if ((char)c != '\n') {
		cin >> tcp_timeout;
	}
			
	cin.get();
	pSettings->Set(tcp_timeout, translation_time_tcp);
	
	cout << "Enter the UDP translation timeout in seconds [" << udp_timeout << "]: ";
	
	c = cin.peek();
			
	if ((char)c != '\n') {
		cin >> udp_timeout;
	}
			
	cin.get();
	pSettings->Set(udp_timeout, translation_time_udp);
	
	cout << "Enter the ICMP translation timeout in seconds [" << icmp_timeout << "]: ";
	
	c = cin.peek();
			
	if ((char)c != '\n') {
		cin >> icmp_timeout;
	}
			
	cin.get();
	pSettings->Set(icmp_timeout, translation_time_icmp);
	
	cout << endl << "Enter the IPv6 prefix that will be used as the destination for translations." << endl;
	cout << "prefix [2000:ffff::]: ";
	
	string prefix;
	
	c = cin.peek();
			
	if ((char)c != '\n') {
		cin >> prefix;
	} else
		prefix = "2000:ffff::";	
	
	cin.get();
	
	in6_addr v6_prefix;
	inet_pton(AF_INET6, prefix.c_str(), &v6_prefix);
	pSettings->Set(v6_prefix, ipv6_prefix);
	
	cout << endl << "Please enter the IPv4 address of the DNS server you are currently using." << endl;
	cout << "IPv4 DNS server: ";
	
	string dns_server;
	cin >> dns_server;
	cin.get();
	
	cout << endl << "You can configure hosts for automatic DNS translation by using the DNS server below." << endl;
	in_addr v4_dns;
	inet_pton(AF_INET, dns_server.c_str(), &v4_dns);
	
	memcpy((char*)&v6_prefix + 12, &v4_dns, sizeof(in_addr));
	char buffer[INET6_ADDRSTRLEN];
	cout << "IPv6 DNS Server: " << inet_ntop(AF_INET6, &v6_prefix, buffer, INET6_ADDRSTRLEN) << endl;
	
	
	cout << endl << "Thank you for choosing Ataga as you IPv4/IPv6 NAT-PT solution." << endl;
	cout << "Setup is now complete. Type 'naptd' to start NAT-PT." << endl << endl;
	
	clean_up(0);
	
	return 0;
}

void print_usage(void)
{
	puts("Ataga IPv4/IPv6 NAT-PT Configuration Maker");
	puts("(c) 2005 by Lukasz Tomicki <lucas.tomicki@gmail.com>");
	puts(" -h display this help");
	puts(" -c [path] create configuration file in non-default location");
	puts("    (default: /etc/naptd.conf)");
}

void get_all(istream& stream, char last)
{
	char c(last);
	while (!stream.eof() && c != 10)
		c = stream.get();
}

void clean_up(int code)
{
	delete pSettings;	
	exit(code);
}
