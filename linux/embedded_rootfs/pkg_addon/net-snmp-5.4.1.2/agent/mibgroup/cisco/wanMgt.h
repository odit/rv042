
FindVarMethod var_wanMgt;
FindVarMethod var_wanConnectionTable;

FindVarMethod var_wanDnsNetAddressTable;

FindVarMethod var_wan1DnsNetAddressTable;
FindVarMethod var_wan2DnsNetAddressTable;


WriteMethod write_wanConnectionType;
WriteMethod write_wanNetAddress;
WriteMethod write_wanPhysicalAddr;
WriteMethod write_wanSubnetMask;
WriteMethod write_wanDefaultGateway;
WriteMethod write_wanDHCPStatus;
WriteMethod write_wanLoginStatus;
WriteMethod write_wanLoginUserName;
WriteMethod write_wanLoginPassword;
WriteMethod write_wanWorkingMode;
WriteMethod write_wanConnectedState;
WriteMethod write_wanConnectedIdleTime;
WriteMethod write_wanConnectedRedialPeriod;
WriteMethod write_wanDnsAutoNegoStatus;
WriteMethod write_wanDnsNetAddress;

WriteMethod write_wan1DnsNetAddress;
WriteMethod write_wan2DnsNetAddress;


#define   WANINDEX              3
#define   WANIFINDEX            4
#define   WANCONNECTIONTYPE     5
#define   WANNETADDRESS         6
#define   WANPHYSICALADDR       7
#define   WANSUBNETMASK         8
#define   WANDEFAULTGATEWAY     9
#define   WANDHCPSTATUS         10
#define	  WANLOGINSTATUS				11
#define   WANLOGINUSERNAME      12
#define   WANLOGINPASSWORD      13
#define   WANWORKINGMODE        15
#define   WANCONNECTEDSTATE     16
#define   WANCONNECTEDIDLETIME  17
#define   WANCONNECTEDREDIALPERIOD  18
#define   WANDNSAUTONEGOSTATUS  19
#define   WANDNSNETADDRESSINDEX  21
#define   WANDNSIFINDEX         22
#define   WANDNSNETADDRESS      23

#define   WAN1DNSNETADDRESSINDEX  121
#define   WAN1DNSIFINDEX         122
#define   WAN1DNSNETADDRESS      123
#define   WAN2DNSNETADDRESSINDEX  124
#define   WAN2DNSIFINDEX         125
#define   WAN2DNSNETADDRESS      126
