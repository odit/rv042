# contents below built automatically by configure; do not edit by hand
mib_module_list_lo= \
	snmpv3/snmpEngine.lo \
	snmpv3/snmpMPDStats.lo \
	snmpv3/usmStats.lo \
	snmpv3/usmUser.lo \
	mibII/system_mib.lo \
	mibII/sysORTable.lo \
	mibII/at.lo \
	mibII/ip.lo \
	mibII/snmp_mib.lo \
	mibII/tcp.lo \
	mibII/icmp.lo \
	mibII/udp.lo \
	mibII/vacm_vars.lo \
	mibII/setSerialNo.lo \
	ucd-snmp/proc.lo \
	ucd-snmp/versioninfo.lo \
	ucd-snmp/pass.lo \
	ucd-snmp/pass_persist.lo \
	ucd-snmp/disk.lo \
	ucd-snmp/loadave.lo \
	agent/extend.lo \
	ucd-snmp/errormib.lo \
	ucd-snmp/file.lo \
	ucd-snmp/dlmod.lo \
	ucd-snmp/proxy.lo \
	ucd-snmp/logmatch.lo \
	ucd-snmp/memory.lo \
	ucd-snmp/vmstat.lo \
	notification/snmpNotifyTable.lo \
	notification/snmpNotifyFilterProfileTable.lo \
	notification-log-mib/notification_log.lo \
	target/snmpTargetAddrEntry.lo \
	target/snmpTargetParamsEntry.lo \
	target/target.lo \
	target/target_counters.lo \
	agent/nsTransactionTable.lo \
	agent/nsModuleTable.lo \
	agent/nsDebug.lo \
	agent/nsCache.lo \
	agent/nsLogging.lo \
	agent/nsVacmAccessTable.lo \
	disman/event/mteScalars.lo \
	disman/event/mteTrigger.lo \
	disman/event/mteTriggerTable.lo \
	disman/event/mteTriggerDeltaTable.lo \
	disman/event/mteTriggerExistenceTable.lo \
	disman/event/mteTriggerBooleanTable.lo \
	disman/event/mteTriggerThresholdTable.lo \
	disman/event/mteTriggerConf.lo \
	disman/event/mteEvent.lo \
	disman/event/mteEventTable.lo \
	disman/event/mteEventSetTable.lo \
	disman/event/mteEventNotificationTable.lo \
	disman/event/mteEventConf.lo \
	disman/event/mteObjects.lo \
	disman/event/mteObjectsTable.lo \
	disman/event/mteObjectsConf.lo \
	disman/schedule/schedCore.lo \
	disman/schedule/schedConf.lo \
	disman/schedule/schedTable.lo \
	utilities/override.lo \
	host/hr_system.lo \
	host/hr_storage.lo \
	host/hr_device.lo \
	host/hr_other.lo \
	host/hr_proc.lo \
	host/hr_network.lo \
	host/hr_print.lo \
	host/hr_disk.lo \
	host/hr_partition.lo \
	host/hr_filesys.lo \
	host/hr_swrun.lo \
	host/hr_swinst.lo \
	util_funcs.lo \
	mibII/kernel_linux.lo \
	mibII/ipAddr.lo \
	mibII/var_route.lo \
	mibII/route_write.lo \
	mibII/tcpTable.lo \
	mibII/udpTable.lo \
	mibII/vacm_context.lo \
	ip-mib/ip_scalars.lo \
	header_complex.lo \
	snmp-notification-mib/snmpNotifyFilterTable/snmpNotifyFilterTable.lo \
	if-mib/ifTable/ifTable.lo \
	if-mib/ifXTable/ifXTable.lo \
	ip-mib/ipAddressTable/ipAddressTable.lo \
	ip-mib/inetNetToMediaTable/inetNetToMediaTable.lo \
	ip-mib/inetNetToMediaTable/inetNetToMediaTable_interface.lo \
	ip-mib/inetNetToMediaTable/inetNetToMediaTable_data_access.lo \
	ip-mib/ipSystemStatsTable/ipSystemStatsTable.lo \
	ip-mib/ipSystemStatsTable/ipSystemStatsTable_interface.lo \
	ip-mib/ipSystemStatsTable/ipSystemStatsTable_data_access.lo \
	ip-forward-mib/ipCidrRouteTable/ipCidrRouteTable.lo \
	ip-forward-mib/inetCidrRouteTable/inetCidrRouteTable.lo \
	tcp-mib/tcpConnectionTable/tcpConnectionTable.lo \
	tcp-mib/tcpListenerTable/tcpListenerTable.lo \
	udp-mib/udpEndpointTable/udpEndpointTable.lo \
	hardware/memory/hw_mem.lo \
	hardware/memory/memory_linux.lo \
	hardware/cpu/cpu.lo \
	hardware/cpu/cpu_linux.lo \
	snmp-notification-mib/snmpNotifyFilterTable/snmpNotifyFilterTable_interface.lo \
	snmp-notification-mib/snmpNotifyFilterTable/snmpNotifyFilterTable_data_access.lo \
	if-mib/data_access/interface.lo \
	if-mib/ifTable/ifTable_interface.lo \
	if-mib/ifTable/ifTable_data_access.lo \
	if-mib/ifXTable/ifXTable_interface.lo \
	if-mib/ifXTable/ifXTable_data_access.lo \
	ip-mib/ipAddressTable/ipAddressTable_interface.lo \
	ip-mib/ipAddressTable/ipAddressTable_data_access.lo \
	ip-mib/data_access/arp_common.lo \
	ip-mib/data_access/arp_linux.lo \
	ip-mib/data_access/systemstats_common.lo \
	ip-mib/data_access/systemstats_linux.lo \
	ip-mib/data_access/scalars_linux.lo \
	ip-forward-mib/ipCidrRouteTable/ipCidrRouteTable_interface.lo \
	ip-forward-mib/ipCidrRouteTable/ipCidrRouteTable_data_access.lo \
	ip-forward-mib/inetCidrRouteTable/inetCidrRouteTable_interface.lo \
	ip-forward-mib/inetCidrRouteTable/inetCidrRouteTable_data_access.lo \
	tcp-mib/data_access/tcpConn_common.lo \
	tcp-mib/data_access/tcpConn_linux.lo \
	tcp-mib/tcpConnectionTable/tcpConnectionTable_interface.lo \
	tcp-mib/tcpConnectionTable/tcpConnectionTable_data_access.lo \
	tcp-mib/tcpListenerTable/tcpListenerTable_interface.lo \
	tcp-mib/tcpListenerTable/tcpListenerTable_data_access.lo \
	udp-mib/udpEndpointTable/udpEndpointTable_interface.lo \
	udp-mib/udpEndpointTable/udpEndpointTable_data_access.lo \
	if-mib/data_access/interface_linux.lo \
	if-mib/data_access/interface_ioctl.lo \
	ip-mib/data_access/ipaddress_common.lo \
	ip-mib/data_access/ipaddress_linux.lo \
	ip-forward-mib/data_access/route_common.lo \
	ip-forward-mib/data_access/route_linux.lo \
	ip-forward-mib/data_access/route_ioctl.lo \
	udp-mib/data_access/udp_endpoint_common.lo \
	udp-mib/data_access/udp_endpoint_linux.lo \
	ip-mib/data_access/ipaddress_ioctl.lo

# end configure generated code
