
#include <errno.h>
#include <signal.h>
#include <string>
#include <iostream>
#include <linux/if_packet.h>
#include <netinet/ip.h>

using namespace std;

in6_addr 		v6_prefix;


__LP64__


int main(int argc, char** argv)
{
	int a;
	long long l;

	cout << "int: " << sizeof(a) << endl;
	cout << "long long: " << sizeof(l) << endl;
	cout << "in6_addr: " << sizeof(v6_prefix) << endl;
	cout << "in6_addr addr: " << sizeof(&v6_prefix) << endl;
}
