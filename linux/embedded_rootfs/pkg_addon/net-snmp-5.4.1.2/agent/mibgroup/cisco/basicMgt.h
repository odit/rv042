
FindVarMethod var_basicMgt;
FindVarMethod var_mgtCommunityTable;


WriteMethod write_mgtSystemReset;
WriteMethod write_mgtFactoryReset;
WriteMethod write_mgtAdministrator;
WriteMethod write_mgtAdminUsername;
WriteMethod write_mgtHostName;
WriteMethod write_mgtDomainName;
WriteMethod write_mgtNodeNetAddress;
WriteMethod write_mgtNodeSubnetMask;
WriteMethod write_mgtDhcpStatus;
WriteMethod write_mgtDhcpStartNetAddr;
WriteMethod write_mgtDhcpNumberUsers;
WriteMethod write_mgtCommunityName;
WriteMethod write_mgtCommunityType;

#define   MGTSYSTEMRESET        1
#define   MGTFACTORYRESET       2
#define   MGTADMINISTRATOR      3
#define   MGTADMINUSERNAME      4
#define   MGTHOSTNAME           6
#define   MGTDOMAINNAME         7
#define   MGTNODENETADDRESS     8
#define   MGTNODESUBNETMASK     9
#define   MGTDHCPSTATUS         10
#define   MGTDHCPSTARTNETADDR   11
#define   MGTDHCPNUMBERUSERS    12
#define   MGTCOMMUNITYINDEX     15
#define   MGTCOMMUNITYNAME      16
#define   MGTCOMMUNITYTYPE      17