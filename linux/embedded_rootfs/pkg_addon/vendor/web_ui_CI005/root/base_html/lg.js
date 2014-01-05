/******************** alert message ********************/
var aDNSServer="Please input DNS Server !";
var aDmzrangeIFaceConfict="The function of DMZ Range can only be used when using Static IP. Please change WAN1 connection type to Static IP.";
var aInputInterface="Please choose an Interface.";
var aPolicyCheck="The rule setting has conflict with existing Strategy rule.";
/*var aCheckName="Some special characters or symbols are not supported, including the space between two characters.\nPlease modify your setting! ";*/
var aCheckName="Three special characters are not supported: Backslash(\\), double quote(\"), and single quote(\'). ";
var addtoListMessage1="Please input interface";

var aIBindConflict="The Interface Binding function in advanced conflicts with tunnel";
var aSi_t="!";
var aBKTimerCheck="VPN Backup Failover Timer is out of range [30~999] seconds !";
var aEchoIntervalCheck=" The value (Second) is out of range [5~9999] !";
var aEchoFailureCheck="The value (Time) is out of range [1~99] !";

/* access_rules.htm */
var aLimitRules="The maximum number of Access Rule entries is ";
var aLimitEnd=". \nYou can't add any more.";
var aLimitRule="The maximum number of Rule entries is ";

/* adv_forwarding.htm */
var aLimitForwarding="The maximum number of Port Range Forwarding entries is ";
var aLimitTriggering="The maximum number of Port Triggering entries is ";
var aApName="Please input Application Name !";
var aTriggerPortRange="Please input Trigger Port Range !";
var aIncomePortRange="Please input Incoming Port Range !";
var aTriggerPortAlready="Some Ports in the Trigger Port Range were already in the list. You cannot add it to the list again.";
var aIncomePortAlready="Some Ports in the Incoming Port Range were already in the list. You cannot add it to the list again.";
var aServiceAlready="This Service was already in the list. You cannot add it to the list again.";

/* adv_mac.htm */
var aSameMAC="WAN2 MAC Address must be different from WAN1 MAC Address !";
var aLANMAC="LAN MAC Address must be different from WAN MAC Address !";
var aWanLanMAC="WAN MAC Address must be different from LAN MAC Address !";
var aWanDmzMAC="WAN MAC Address must be different from DMZ MAC Address !";
var aWan1LanMAC="WAN1 MAC Address must be different from LAN MAC Address !";
var aWan2LanMAC="WAN2 MAC Address must be different from LAN MAC Address !";

/* adv_nat.htm */
var aDMZinPub="DMZ Port IP Address was in the Public IP Range. You can't add any more.";
var aDMZinPri="DMZ Port IP Address was in the Private IP Range. You can't add any more.";
var aLANinPri="LAN Port IP Address was in the Private IP Range. You can't add any more !";
var aWAN1inPri="WAN1 Port IP Address was in the Private IP Range. You can't add any more !";
var aWAN2inPri="WAN2 Port IP Address was in the Private IP Range. You can't add any more !";
var aWAN3inPri="WAN3 Port IP Address was in the Private IP Range. You can't add any more !";
var aWAN4inPri="WAN4 Port IP Address was in the Private IP Range. You can't add any more !";
var aWAN5inPri="WAN5 Port IP Address was in the Private IP Range. You can't add any more !";
var aWAN6inPri="WAN6 Port IP Address was in the Private IP Range. You can't add any more !";
var aWAN7inPri="WAN7 Port IP Address was in the Private IP Range. You can't add any more !";
var aLANinPub="LAN Port IP Address was in the Public IP Range. You can't add any more !";
var aWAN1inPub="WAN1 Port IP Address was in the Public IP Range. You can't add any more !";
var aWAN2inPub="WAN2 Port IP Address was in the Public IP Range. You can't add any more !";
var aWAN3inPub="WAN3 Port IP Address was in the Public IP Range. You can't add any more !";
var aWAN4inPub="WAN4 Port IP Address was in the Public IP Range. You can't add any more !";
var aWAN5inPub="WAN5 Port IP Address was in the Public IP Range. You can't add any more !";
var aWAN6inPub="WAN6 Port IP Address was in the Public IP Range. You can't add any more !";
var aWAN7inPub="WAN7 Port IP Address was in the Public IP Range. You can't add any more !";
var aIPAddress="Please input IP Address first !";
var aIPAddressPri="Please input IP Address (Private Range Begin) !";
var aIPAddressPub="Please input IP Address (Public Range Begin) !";
var aRangeLength="Please input Range Length !";
var aRangeLengthCheck="Range Length is out of range !";
var aLimitNAT="The maximum number of One-to-One NAT entries is ";
var aPrivateIPAlready="Some Private IP Addresses in the range were already in the list. You cannot add it to the list again.";
var aPublicIPAlready="Some Public IP Addresses in the range were already in the list. You cannot add it to the list again.";

/* adv_routing.htm */
var aHopCount="Please Input Hop Count.";
var aLimitRouting="The maximum number of Static Routing entries is ";
var aHopCountCheck="Hop Count is out of range [1~15] !";
var aDestIPAlready="This Destination IP was already in the list. You cannot add it to the list again.";

/* adv_upnp.htm */
var aNameIP="Please input Name or IP Address.";
var aLimitUPnP="The maximum number of UPnP Forwarding entries is ";

/* client_to_gateway_g.htm */
var aP2AuthEncthesame="Both NULLs in the drop-down menu of Phase 2 Encryption and Phase 2 Authentication should not be selected simultaneously.Please change one of them.";
var aSALifeTimeCheck="SA Life Time is out of range [120~86400] seconds !";
var aSALifeTime2Check="SA Life Time is out of range [120~28800] seconds !";
var aGroupName="Please input Group Name !";
var aIPRangeLSG="Please Input IP range of Local Security Group !";
var aUserDomainName="Please input User Domain Name !";
var aPresharedKey="Please input Preshared Key !";
var aP1SALifeTime="Please input Phase 1 SA Life Time !";
var aP2SALifeTime="Please input Phase 2 SA Life Time !";

/* client_to_gateway_t.htm */
var aTunnelName="Please input Tunnel Name !";
var aIPAddressRemote="Please input IP address of Remote Client !";
var aIPRemoteCheck="IP address of Remote Client can not be 0.0.0.0 !";
var aIncomeSPI="Please input Incoming SPI !";
var aOutgoSPI="Please input Outgoing SPI !";
var aEncryptionKey="Please input Encryption Key !";
var aAuthenticationKey="Please input Authentication Key !";
var aEncryptionKeyCheck="The Encryption Key must be a HEX number !";
var aAuthenticationKeyCheck="The Authentication Key must be a HEX number !";
var aIncomeSPIHexCheck="Incoming SPI must be a HEX number !";
var aOutgoSPIHexCheck="Outgoing SPI must be a HEX number !";
var aIncomeSPICheck="Incoming SPI must be a HEX number in the range of 100 to ffffffff !";
var aOutgoSPICheck="Outgoing SPI must be a HEX number in the range of 100 to ffffffff !";
var aDPDITime="DPD Interval Time is out of range [10~999] seconds !";

/*edit_sys_dualwan2.htm*/
var aDesIPCheck="Please Input IP range of Destination IP!";
var aSourceIPCheck="Please input Source IP Range !";
var aDNSHostSame="The DNS Lookup Host Name for each WAN port should be different. Please enter it again."; // 2004/11/18 Eric	
var aIOIPCheck="Please do not type domain in this field ; Please enter IP address name ex.\"192.168.1.2\" again.";
var aDomainCheck="Please do not type IP address in this field ; Please enter domain name ex.\"www\" again.";
var aIPRangeSrc="Please input Source IP Range !";
var aLimitProtocolBinding="The maximum number of Protocol Binding entries is ";
var aLimitIPGroup="The maximum number of IP Group entries is ";

var aAllTraffic="When \"All Traffic\", Source IP and Destination IP are configured as zero, all services will be directed to the specified WAN interface without going through other WAN ports. Please enter either Source IP or Destination IP.";
var aQosAllTraffic="When \"All Traffic\" and IP are configured as zero, all services will be directed to the specified WAN interface without going through other WAN ports.";

/* gateway_to_gateway.htm */
var aDisableTunnel="You cannot disable the tunnel which is being created.";
var aIPAddressRSW="Please Input IP address of Remote Security Gateway !";
var aIPRSWCheck="IP address of Remote Security Gateway can not be 0.0.0.0 !";
var aIPAddressRSG="Please Input IP address of Remote Security Group !";
var aIPRangeRSG="Please Input IP range of Remote Security Group !";
var aBKPDP="Tunnel Backup function must go with Dead Peer Detection (DPD)!";
var aBKTimerCheck="VPN Backup Failover Timer is out of range [30~999] seconds!";

/*===================================================================*/

var aDPDTimeCheck="DPD Interval Time is out of range [10~999] seconds !";
var aBKDPD="Tunnel Backup function must be in Dead Peer Detection (DPD) mode !";
var aBKSmartLink="Warnning !!You need to change WAN Routing Mode from Smart Link Backup to Auto Mode.";

/*===================================================================*/

/* DNS Resolved */
var aResolvedName="Please input DNS Resolve Name !";

/* content_filter.htm */
var aForbiddenDomain="Please input Forbidden Domain.";
var aLimitForbiddenDomains="The maximum number of Forbidden Domains entries is ";
var aForbiddenDomainAlready="This Forbidden Domain was already in the list. You cannot add it to the list again.";
var aAllowedDomain="Please input Allowed Domain.";
var aLimitAllowedDomains="The maximum number of Allowed Domains entries is ";
var aAllowedDomainAlready="This Allowed Domain was already in the list. You cannot add it to the list again.";
var aIPcfCheck="IP address range can not be 0.0.0.0~0 !";
var aLimitExceptionIP="The maximum number of Exception IP entries is ";
var aLimitExceptionRule="The maximum number of Exception rule entries is ";

/*===========================================================*/

var sUpdateKeyword="Update";
var aKeyword="Please input Keyword.";
var aLimitKeywords="The maximum number of Keywords entries is ";
var aLimitKeyword="This Keyword was already in the list. You cannot add it to the list again.";

/*===================================================================*/

/* dhcp_setup.htm */
var aDhcpLanIpConflict="DHCP IP Address Range conflict with the LAN IP.";
var aDhcpReplyServerIPConflict="DHCP Relay Server IP Address conflict with the LAN IP.";
var aIPAddressStart="Please input IP Address (Range Start) !";
var aIPAddressEnd="Please input IP Address (Range End) !";
var aDhcpServer="Once you enable DHCP Server, the DHCP Relay will be disabled. Are you sure you want to enable DHCP Server now?";
var aDhcpRelay="Once you enable  DHCP Relay, the DHCP Server will be disabled. Are you sure you want to enable DHCP Relay now?";

/* dhcp_status.htm */
var cRemoveClient="Are you sure you want to remove this client right now?";

/* ip_mac_binding.htm*/
var aWaitForSetting="It will take some time to complete,please don't reclick the page or relogin it, it will make system busy or shut down !!";

/*===================================================================*/

var aDeviceIPAlready="The IP Address was Device IP Address. You cannot add it to the list.";
var aIPAlready="The IP Address was already in the list!";
var aMACAlready="The MAC Address was already in the list!";
var aLimitStaticIP="The maximum number of Static IP entries is ";
var aGroupExist =" The group already exists!!";

/*===================================================================*/

/* edit_access_rules.htm */
var aTimeSchedule="Please input Time Schedule.";

/*edit_sys_dualwan2.htm*/
var aIP0to254Value="IP value is out of range [0~254]";

/*edit_network.htm*/
var aConflictIP="The IP Address setting has conflict with existing interface.";
var aInternalConflictWAN1="This IP Address Range conflict with the WAN1 Internal LAN IP range.";
var aInternalConflictWAN2="This IP Address Range conflict with the WAN2 Internal LAN IP range.";
var aInternalConflictWAN3="This IP Address Range conflict with the WAN3 Internal LAN IP range.";
var aInternalConflictWAN4="This IP Address Range conflict with the WAN4 Internal LAN IP range.";
var aInternalConflictWAN5="This IP Address Range conflict with the WAN5 Internal LAN IP range.";
var aInternalConflictWAN6="This IP Address Range conflict with the WAN6 Internal LAN IP range.";
var aInternalConflictWAN7="This IP Address Range conflict with the WAN7 Internal LAN IP range.";
var aInternalConflictWAN8="This IP Address Range conflict with the WAN8 Internal LAN IP range.";
var aTwoBridges="You can not select two transparent bridges at the same time.";

/* f_general.htm */
var aBytesCheck="Value is out of range [68~1500]";
var aArpCheck="The number users enter should be between 1~100. Please enter it again.";
var aCheckResetSetting="Are you sure to reset the setting ??";
var aLimitIPSrule_front="The maximum number of entries is ";
var aLimitIPSrule_back=". \nYou can't add any more";
var aLimitAVrule_front="The maximum number of entries is ";
var aLimitAVrule_back=". \nYou can't add any more";
var aLimitwhiterule_front="The maximum number of entries is ";
var aLimitwhiterule_back=". \nYou can't add any more";
var aLimitblackrule_front="The maximum number of entries is ";
var aLimitblackrule_back=". \nYou can't add any more";
var aCheckInfEnable="Please choose at least one interface!";
var aThresholdNumCheck="The Number is out of range [1~100] !";

/*===================================================================*/

var aTrustedDomain="Please input Trusted Domain.";
var aTrustedDomainAlready="This Trusted Domain was already in the list. You cannot add it to the list again.";
var aLimitTrustedDomain="The maximum number of Trusted Domains entries is ";

/*===================================================================*/

/* lan_setting.htm */
var aDisableLANPort="You can not disable ALL LAN ports at the same time.";
var aDisableLANPort1="If you change the number of WAN ports to ";
var aDisableLANPort2=", all LAN ports will be disabled.\nPlease change the Disable setting of Lan ports.";
var aDisableAllPort="You can not disable ALL ports at the same time.";

/* log_setting.htm */
var aMailServer="Please input Mail Server .";
var aEMail="Please input Email Address .";
var aLogServer="Please input Domain Name or IP Address of Log Server !";
var aLogQueueCheck="The value (Log Queue Length) is out of range [10~100] !";
var aLogTimeCheck="The value (Log Time Threshold) is out of range [10~10080] !";
var aEMailCheck="Email address is error!";
var aNoHostName="No Host Name found!\nPlease visit \"Setup->Network\" page and assign a name in 'Host Name' field.";

/* network.htm */
var aDMZSubnetConflict="DMZ Subnet conflicts with the Subnet of LAN.";
var aDMZSubnetRangeConflict = "DMZ Range conflicts with the Specify WAN IP Address.";
var aNoDNS="No DNS Servers entered might cause name resolution problems.";
var aDualWAN="Router has been changed to Dual WAN mode, it will take effect after you save settings.";
var aDMZMode="Router has been changed to DMZ mode, it will take effect after you save settings.";
var aLimitMLan="The maximum number of Multiple Subnet entries is ";
var aChangedSubnet="Configuring specific settings of subnet mask may effect some functionality.";
var aDmzRangeStaticWan="The IP Range must be static public IP.";
var aLanWanSameNetwork="The IP Address and WAN IP should be at the same subnet."; 
var aCheckHostName="The Host Name only can use english characters, numbers, and \"-\".";
var aCheckHostNameFirstLast ="The first or last character of hostname can not be \"-\"."; 
var aDHCPMessage="Prefix Length is bigger than 116. DHCPv6 may not work normally. Press \"OK\" to continue, or press \"Cancel\" to correct it.";
var aLANIPSubnetConflict ="LAN IP conflict with DHCP IP Address Range.";

/* password.htm */
var aNewPasswordFirst="Please input new password first !";
var aNewPassword="Please input new password !";
var aOldPassword="Please input old password !";
var aConfirmNewPassword="Please input confirm new password !";
var aNewUsername="Please input new username!";
var aConfirmNewUsername="Please input confirm new username!";
var aPasswordStrengthWeak="Password strength is not sufficient.  Please enter a new password.";
var aOldPasswordStrengthWeak="Old password strength is not sufficient.  Please change a new password."; 
var aPasswordUsernameEqual="Password cannot be the same as Username.  Please enter a new password.";
var aPasswordNewOldEqual="New Password cannot be the same as Old Password.  Please enter a new password.";
var aNewPasswordNoMatch="New Password and Confirm New Password do not match.  Please re-enter.";
var aNewUsernameNoMatch="New Username and Confirm New Username do not match.  Please re-enter.";
var aUserFormate="~ ` ! @ # $ % ^ & * ( ) - _ + = | \\ / { } [ ] : ; \" \' < > , . ?\nand space between two characters can not be used for username.";

/* service0.htm */
var aServiceName="Please input Service Name !";
var aPortRange="Please input Port Range !";
var aPortInternal="Please input Internal Port !";
var aProtocolPortAlready="Some Ports in the Port Range of this Protocol were already in the list.\nYou cannot add it to the list again.";
var aLimitService="The maximum number of Service entries is ";
var aProtocolPortInput="Please input Procotol Field number.";
var aProtocolFieldSame="The value has already exist, please enter a new value in the field of the protocol.";
var aProtocolValue="The value (Procotol Field number) is out of range [1~255] !"; 

/* service1.htm */
var aPortExternal="Please input External Port !";
var aExternalPortAlready="The External Port of this Protocol was already in the list.\nYou cannot add it to the list again.";

/* sys_dualwan2.htm */
var aStreamCheck="The value (Upstream or Downstream) is out of range [1~100000] !";

/*==============for NonBrand=============================*/

var aCountCheck="The value (Retry count) is out of range [1~99999] !";
var aTimeoutCheck="The value (Retry timeout) is out of range [1~9999999] !";
var aNSDwan1="Please check a method to detect network service in WAN1 !";
var aNSDwan2="Please check a method to detect network service in WAN2 !";
var aISPHostwan1="Please input ISP Host in WAN1 !";
var aISPHostwan2="Please input ISP Host in WAN2 !";
var aRemoteHostwan1="Please input Remote Host in WAN1 !";
var aRemoteHostwan2="Please input Remote Host in WAN2 !";
var aDNSHostwan1="Please input DNS Lookup Host in WAN1 !";
var aDNSHostwan2="Please input DNS Lookup Host in WAN2 !";

/*==============================================*/

var aNSDwan="Please check a method to detect network service.";
var aISPHostwan="Please input ISP Host.";
var aRemoteHostwan="Please input Remote Host.";
var aDNSHostwan="Please input DNS Lookup Host.";

/* qos.htm */
var aDuplicatedEntry="Duplicated entry is already in the list! You can't add any more.";
var aOneMoreQosWan="Please choose at least one wan ports!";

/* sys_firmware.htm */
var aSelectFile="Please select a file first!";

/* sys_snmp.htm */
var aSNMPToBroadcast="You cannot send SNMP trap to broadcast IP address.";
var aCheckSpecialCharacter="Special characters are not accepted";
var aCheckSNMPName="SNMP System Name only can use english characters, numbers, and \"-_\".";
var aCheckSNMPNameFirstLast ="The first or last character of SNMP System Name can not be \"-_\".";
var aCheckSNMPContact="SNMP System Contact only can use english characters, numbers, and \"-_\".";
var aCheckSNMPContactFirstLast ="The first or last character of SNMP System Contact can not be \"-_\".";
var aCheckSNMPLocation="SNMP System Location only can use english characters, numbers, and \"-_\".";
var aCheckSNMPLocationFirstLast ="The first or last character of SNMP System Location can not be \"-_\".";

/* time2.htm */
var aHours="Please input Hours !";
var aMinutes="Please input Minutes !";
var aSeconds="Please input Seconds !";
var aMonth="Please input Month !";
var aDay="Please input Day !";
var aYear="Please input Year !";

/* wizard_basic.htm */
var aRouteWAN="The Static Route setting is using WAN";
var aModify=". Please modify it first !";
var aVPNWAN="The VPN setting is using WAN";
var aRuleWAN="The Access Rules setting is using WAN";
var aGatewayIPAddr="Please input Default Gateway IP Address.";
var aIdleTime="Please input Max. Idle Time.";
var aRedialPeriod="Please input Redial period.";
var aIPAddressDMZ="Please input DMZ IP address.";
var aIPRangeDMZ="Please input DMZ Public IP Range.";

/* BadFile.htm */
var aBadFile=" Bad upgrade file !  Please input a correct file !";

/*=======================================================================*/

/* isp.htm */

var aISPName="Please input ISP Name !";
var aUserID="Please input User ID !";
var aIdleTime="Please input Max Idle Time !";
var aRedialTime="Please input Redial Period !";
var aLimitISP="The maximum number of ISP entries is ";
var aDestAddress="Please input Destination Address !";

/*=======================================================================*/
/* qos.htm */ 
var aRateKbitCheck="The value (Rate) is out of range [1~99999] !";
var aLimitQosRate="The maximum number of QoS entries is 100. \nYou can't add any more.";
var aLimitQosPrio="The maximum number of QoS entries is 50. \nYou can't add any more.";
var aLimitQosStream="User setting is over the enabled WAN capacity: ";
var aLimitMinRate="User setting (Mini. Rate in the list) is over the enabled WAN";
var aLimitMaxRate="User setting (Max. Rate) is over the enabled WAN";
var aRateEnd=" capacity:";
var aDuplicatedEntry="Duplicated entry is already in the list! You can't add any more.";
var aMiniMaxRateCheck="The value of Minimum Rate is over the value of Maxmum Rate !";
var aMinRate="Please input Mini. Rate !";
var aMaxRate="Please input Max. Rate !";
var aIPRange="Please input IP Range !";

var aUserName="Please input Username !";
var aPassword="Please input Password !";
var aHostName="Please input Host Name !";
var aDomainName="Please input Domain Name !";
var aPressService="Press \"Service Management\" button, and add a new service first.";
var aNetworkRangeCheck="IP Address is out of range ";
var aIPCheck="IP Address is out of range [0~255] !";
var aIOCheck ="The value is out of range [1%~10%]!";
var aIP0to254Check="IP Address is out of range [0~254] !";
var aIP1to254Check="IP Address is out of range [1~254] !";
var aPortCheck="Port Number is out of range [1~65535] !";
var aPort0Check="Port Number is out of range [0~65535] !";
var aPort2to65534Check="Port Number is out of range [2~65534] !";
var aMACCheck="This MAC Address is out of range [00~ff] !";
var aIPAddress="Please input IP Address.";
var aIPAddressWAN="Please Input WAN IP Address.";
var aMAC="Please Input MAC Address.";
var aMask="Please Input Subnet Mask.";
var aGateway="Please Input Default Gateway.";
var aMaskCheck="Subnet Mask is out of range [0~255] !";
var aHourCheck="The value (Hour) is out of range [0~23] !";
var aMinuteCheck="The value (Minute) is out of range [0~59] !";
var aMinuteSNumsCheck="The value (Minute) is out of range [5~43200] !";
var aMinuteNumsCheck="The value (Minute) is out of range [0~9999] !";
var aMinuteNums2Check="The value (Minute) is out of range [1~99999] !";
var aMinuteNums3Check="The value (Minute) is out of range [1~99] !";
var aPercentageCheck="The value (%) is out of range [0~99] !";
var aSecondCheck="The value (Second) is out of range [0~59] !";
var aMonthCheck="The value (Month) is out of range [1~12] !";
var aDayCheck="The value (Day) is out of range [1~31] !";
var aDay30Check="The value (Day) is out of range [1~30] !";
var aDay28Check="The value (Day) is out of range [1~28] !";
var aDay29Check="The value (Day) is out of range [1~29] !";
var aYearCheck="The value (Year) is out of range [0~9999] !";
var aSecondNumsCheck="The value (Second) is out of range [0~999] !";
var aSecondNums2Check="The value (Second) is out of range [1~999999] !";
var aSecondNums3Check="The value (Second) is out of range [1~9999999] !";
var aCacheSizeCheck="Cache Size is out of range [0~999] KB !";
var aAlready="Already in the list !";
var aIPAddressDes="Please Input Destination IP Address.";
var aIPAddressSrc="Please input Source IP Address.";
var aSchedulTime="Please input Scheduling Time !";
var aForbidenScheduleRule = "This setting will violate scheduling rule[00:00 ~ 23:59]!!";

/* pptp.htm */
var aUserNameAlready="This Username was already in the list. You cannot add it to the list again.";
var aLimitUserNames ="The maximum number of users entries is ";
var aAdminUserName="You cannot input 'admin' as Username.";
var aInvaidUserName="Invaid Username ['{'].";
var aInvaidPasswd="Invaid Password ['********' or ';'].";
var aDhcpRangeConflict="PPTP IP Address Range conflict with the Dynamic IP Range of DHCP Server.";
var aLanIpConflict="PPTP IP Address Range conflict with the LAN IP.";
var aLimitPPTPRange="The max entries in PPTP IP Address Range are 10. \nPlease adjust the PPTP IP Address Range.";
var aLimitPPTPRange2="The max entries in PPTP IP Address Range are 5. \nPlease adjust the PPTP IP Address Range.";
/* vpn_clients.htm */
var aLimitClientUsersStart="Only ";
var aLimitClientUsersEnd=" users are allowed. \nYou can't add any more.";
var aCertificateFileNameErr="A file extension name is incorrect. Please make sure it is .pem.";

var aMACDefaultWAN1="This MAC Address must be different from WAN1 Default MAC Address !";
var aMACDefaultWAN2="This MAC Address must be different from WAN2 Default MAC Address !";
var aMACDefaultWAN3="This MAC Address must be different from WAN3 Default MAC Address !";
var aMACDefaultWAN4="This MAC Address must be different from WAN4 Default MAC Address !";
var aMACDefaultWAN5="This MAC Address must be different from WAN5 Default MAC Address !";
var aMACDefaultWAN6="This MAC Address must be different from WAN6 Default MAC Address !";
var aMACDefaultWAN7="This MAC Address must be different from WAN7 Default MAC Address !";

/******************** confirm message ********************/

/* access_rules.htm */
var cChangePriority="Are you sure you want to change the Priority of this Policy right now?";
var cRemoveRules1="Are you sure you want to remove Policy No.";
var cRestoreDefaultRule="Are you sure to return Firewall Access Rule to default? "

/* client_to_gateway_g.htm */
var cGroupVPNOk="Settings are successful. Press 'Ok' to add another Group VPN, or press 'Cancel' to return to the page of VPN Summary.";

/* client_to_gateway_t.htm */
var cClientToGatewayOk="Settings are successful. Press 'Ok' to add another Client to Gateway Tunnel, or press 'Cancel' to return to the page of VPN Summary.";

/* dhcp_setup.htm  ryoko20040728*/
var aDhcpLanIpConflict="DHCP IP Address Range conflict with the LAN IP.";
var aDhcpLanIpOutof="DHCP IP Address Range is out of LAN subnet.";
var cCloseDHCP="Are you sure you want to disable dhcp service right now?\n\n Warrning: It'll reset your LAN setting!!";
var cCloseARP ="Are you sure you want to disable Prevent ARP Virus Attack service?\n\n Warrning: It'll affect IP&MAC Binding!!"

/* dhcp_status.htm */
var cRemoveClient="Are you sure you want to remove this client right now?";

/* edit_access_rules.htm */
var cAccessRuleOk="Settings are successful. Press 'Ok' to add another access rule, or press 'Cancel' to return to the page of Access Rules.";

/*edit_adv_mac.htm*/
var cMACAddrWAN1="This MAC Address must be different from WAN1 MAC Address !";
var cMACAddrWAN2="This MAC Address must be different from WAN2 MAC Address !";
var cMACAddrWAN3="This MAC Address must be different from WAN3 MAC Address !";
var cMACAddrWAN4="This MAC Address must be different from WAN4 MAC Address !";
var cMACAddrWAN5="This MAC Address must be different from WAN5 MAC Address !";
var cMACAddrWAN6="This MAC Address must be different from WAN6 MAC Address !";
var cMACAddrWAN7="This MAC Address must be different from WAN7 MAC Address !";
var cMACAddrDMZ="This MAC Address must be different from DMZ MAC Address !";
var cMACAddrLAN="This MAC Address must be different from LAN MAC Address !";
var cMACClone_one="RV042 supports MAC clone on one WAN only, which is already enabled on the other WAN now.\nPress \"Ok\" to enable MAC clone on this WAN, and return the other WAN to default.";

/* f_general.htm */
var cChangePassword1="When Firewall disabled, the Remote Management feature will be enabled.\nBut the Router is currently set to its default password.\nAs a security measure, you must change the password before the Remote Management feature can be enabled.\nPress 'Ok' to change your password, or press 'Cancel' to leave the Firewall feature enabled.";
var cChangePassword2="The Router is currently set to its default password.\nAs a security measure, you must change the password before the Remote Management feature can be enabled.\nPress 'Ok' to change your password, or press 'Cancel' to leave the Remote Management feature disabled.";
var cChangePassword3="The Router is currently set to its default password.\nAs a security measure, you must change the password before the HTTPS feature can be enabled.\nPress 'Ok' to change your password, or press 'Cancel' to leave the HTTPS feature disabled.";

/* gateway_to_gateway.htm */
var cC2GMode="The tunnel configuration should be set in Client to Gateway mode.\nIf you insist to procceed, this tunnel MAY NOT work!\nPress 'Ok' to save this tunnel anyway, or press 'Cancel' to Client to Gateway mode.";
var cGatewayToGatewayOk="Settings are successful. Press 'Ok' to add another Gateway to Gateway Tunnel, or press 'Cancel' to return to the page of VPN Summary.";

/* Check LSG and RSG network */
var cSameLsgRsg="The settings of Local Security Group conflict with the settings of Remote Security Group. Press 'Ok' to save settings, or press 'Cancel' to undo the changes.";

/*lan_setting.htm*/
var cWANChange="The number of WAN ports was changed. Make sure your network configuration match the new settings. \nPress 'Ok' to save settings, or press 'Cancel' to do nothing.";

/* network.htm */
var cWANportSave="The number of WAN ports was changed. Before you edit WAN ports, you must save settings. \nPress 'Ok' to save settings, or press 'Cancel' to do nothing.";
var cDeviceIP="Device IP address has been modified. You must login to new IP address next time. \nPress 'Ok' to save settings, or press 'Cancel' to undo the changes.";

/*sys_dualwan2.htm*/
var cModeBalanceSave="The Mode of Load Balance was changed. Before you edit interfaces, you must save settings. Press 'Ok' to save settings, or press 'Cancel' to do nothing.";

/* sys_factory.htm */
var cFactoryDefault="Are you sure you want to return to default setting?";

/* sys_firmware.htm */
var cFirmware="Are you sure you want to upgrade firmware right now?";

/* sys_restart.htm */
var cRestart="Are you sure you want to restart router?";

/* sys_setting.htm */
var cConfigImport="Are you sure you want to import configuration file right now?";

/* time1.htm */
var cDay31="The value 31 (Day) is not allowable. Press 'Ok' to modify the value (Day), or press 'Cancel' to modify the value (Month).";
var cDayFeb1="The value ";
var cDayFeb2=" (Day) is not allowable. Press 'Ok' to modify the value (Day), or press 'Cancel' to modify the value (Month).";

/* time2.htm */
var cDayFeb3=" (Day) is not allowable. Press 'Ok' to modify the value (Day), or press 'Cancel' to modify the value (Year).";

/* vpn_summary.htm */
var cRemoveTunnels1="Are you sure you want to remove Tunnel No.";
var cRightNow=" right now?";
var cRemoveGroupVPN1="Are you sure you want to remove Group No.";

/* wizard.htm */
var cFirewallDisabled="Firewall had been disabled, you could not do the Access Rules Setup Wizard. Press 'Ok' to turn the page to Firewall General (if you want to enable firewall), or press 'Cancel' to do nothing.";

/* wizard_basic.htm */
var cWizardBasicOk="Settings are successful. Press 'Ok' to turn the page to Wizard, or press 'Cancel' to turn the page to Network.\nBasic Setup Wizard will be closed after your action.";
var cEditNetwork="Settings are successful. Press 'Ok' to Edit Network Settings, or press 'Cancel' to close Basic Setup Wizard.";

/* wizard_policy.htm */
var cWizardRuleOk="Settings are successful. Press 'Ok' to add another access rule, or press 'Cancel' to close Access Rule Setup Wizard and turn the page to Access Rules.";
/* vpn_clients.htm */
var cChangeLanIp="Would you let the system change the device LAN IP to";
var cCertImport="Are you sure you want to import cerificate file right now?";
var cGenerate="The new certificate will replace the old one. Do you want to continue?";
var cAuto="automatically?";


/******************** window message ********************/

/* access_rules.htm */
var wEnableRules="Enable the policy...";
var wChangePriority="Change the priority...";
var wDisableRules="Disable the policy...";
var wEditRules="Edit the policy...";
var wRemoveRules="Remove the policy...";
var wRestoreRules="Restore to default rules...";

/* adv_ddns.htm */
var wEditInterface="Edit the Interface...";

/* dhcp_status.htm */
var wRemoveClient="Remove the client...";

/* sys_factory.htm */
var wDefaultSetting="Return to default setting...";

/* sys_firmware.htm */
var wUpgradeFirmware="Upgrade firmware...";

/* sys_restart.htm */
var wRestart="Restart router...";

/* sys_setting.htm */
var wConfigImport="Import configuration file...";

/* vpn_summary.htm */
var wEditTunnels="Edit the Tunnel...";
var wRemoveTunnel="Remove the tunnel...";
var wEditGroupVPN="Edit the Group VPN...";
var wRemoveGroupVPN="Remove the Group VPN...";

var wRefresh="Refresh pages...";
var wDownLoad="Download the page...";
var wSave="Save settings...";

/******************** title message ********************/

var tWAN2Port="WAN2 Port Information";
var tDMZPort="DMZ Port Information";
var tWANPort="WAN Port Information";
var tWAN1Port="WAN1 Port Information";

/******************** layout message ********************/
var sAddtoList="Add to list";
var sUpdateAp="Update";
var sUpdateRange="Update";
var sUpdateIP="Update";
var sUpdateDomain="Update";
var sUpdateService="Update";
var sUpdateISP="Update this ISP";
var sUpdateEntry="Update";
var sUpdateUser="Update";
var sActive="Active";
var sInactive="Inactive";

/*===========================================================*/

var sUpdateGroup="Update this Group";
var sUpdataAccount="Update this Account";
var sWait="Waiting...";

/*==================qos.htm ===================================*/

var aLimitSessionLimitRule="The maximum number of Exempted Service Port or IP Address entries is ";
var aRateKbitCheckBegin="The value (Rate) is out of range [";
var aRateKbitCheckEnd="] !";

/*advance_DosSettings.htm*/
var aDosPacketCheck="The value is out of range [10~65534] !";
var aDoSTimeCheck="The value is out of range [1~60] !";

/* log_ipstats.html */
var aipsearchMessage1="Please enable Statistic function first.";
var cSortEnabledMessage2="When users do not use this function, please disable this function to enhance the performance of the device.Please click on OK button to enable this function or click on Cancel button to undo the changes";

/* log_traffic.html */
var cSortEnabledMessage1="When users do not use this function, please disable this function to enhance the performance of the device.\nPlease click on OK button to enable this function or click on Cancel button to undo the changes";

var aCheck_DHCP_Range ="DHCP IP Range should be in either LAN IP Subnet or Multiple Subnet, please change the settings.";
var aCheck_SSL_VP_Range ="SSL IP Range should be in either LAN IP Subnet or Multiple Subnet, please change the settings.";
var aCheck_PPTP_Range ="PPTP IP Range should be in either LAN IP Subnet or Multiple Subnet, please change the settings.";
var aCheck_DHCP_VP_PPTP_Range ="PPTP IP RangeASSL IP RangeADHCP IP Range can not overlap. Please change the settings.";
var aCheck_DHCP_PPTP_Range ="PPTP IP RangeADHCP IP Range can not overlap. Please change the settings.";
var aCheck_StartLargerEnd="Start IP cannot be larger than end IP , please re-enter the values.";

/* network.htm*/
var aLanMACCheck="The LAN MAC Address is out of range [00~ff]!";

/* f_general.htm */
var arpnumCheckMessage="The number users enter should be between 1~100. Please enter it again.";
var aUnable_SSL_use_word = "Can\'t use \'@\' in username";
var aUnable_PPTP_use_word = "Can\'t use \'&\' in username";
var aClick="Please Click ";
var aButton=" Button";
var aName="Please input Name again !";
var ipExist="The ip already exists!!";
var ScheduleNotFinish="Time Schedule Setting is not completed !!";

/*Alert IPmode Message */
var alertMessageBegin="You must set the IP mode to Dual-Stack IP in the Setting > Network Page before configuring ";
var selectMessage=". \nPress 'Ok' to go Setting > Network Page, or press 'Cancel' to do nothing.";
