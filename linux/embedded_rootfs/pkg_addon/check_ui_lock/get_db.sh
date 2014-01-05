
#!/bin/bash
#
case $1 in
	"wan")
	if [ "$2" -eq "4" ] || [ "$2" -eq "5" ]; then
		wan=1;type=4;wan_rt=9;max_wan=10;wan_t=wr;
	else
		wan=1;type=4;wan_rt=12;max_wan=16;wan_t=wr;
	fi
	if [ "$3" -eq "2" ]; then
		t_type="-1 hour";
	fi
	if [ "$3" -eq "3" ]; then
		t_type="-1 day";
	fi
	if [ "$3" -eq "4" ]; then
		t_type="-7 day";
	fi
	while [ "$max_wan" -gt "0" ] 
	do
		if [ "$type" -eq "$wan_rt" ]; then
			wan_t=wt;wan=1;
		fi
		`sqlite -wan /tmp/db.db "select timeEnter,value from t$3 \
		where type=$type and timeEnter >strftime('%s','now','$t_type') \
		order by timeEnter asc" > /tmp/QRTG/$wan_t$wan.inf `;
		wan=$(($wan+1));type=$(($type+1));max_wan=$(($max_wan-1));
	done
	;;
	"wan_avg")
	if [ -e /tmp/QRTG/wraa.inf ]; then
		rm /tmp/QRTG/wraa.inf /tmp/QRTG/wtaa.inf;
	fi
	if [ "$2" -eq "4" ] || [ "$2" -eq "5" ]; then
		wan=1;type=4;wan_rt=9;max_wan=10;wan_t="/tmp/QRTG/wraa.inf";
	else
		wan=1;type=4;wan_rt=12;max_wan=16;wan_t="/tmp/QRTG/wraa.inf";
	fi
	if [ "$3" -eq "2" ]; then
		t_type="-1 hour";
	fi
	if [ "$3" -eq "3" ]; then
		t_type="-1 day";
	fi
	if [ "$3" -eq "4" ]; then
		t_type="-7 day";
	fi
	while [ "$max_wan" -gt "0" ] 
	do
		if [ "$type" -eq "$wan_rt" ]; then
			wan_t="/tmp/QRTG/wtaa.inf";
		fi
		printf `sqlite -avg /tmp/db.db "select avg(value) from t$3 \
		where type=$type and timeEnter >strftime('%s','now','$t_type')"` >> $wan_t;
		wan=$(($wan+1));type=$(($type+1));max_wan=$(($max_wan-1));
	done
	;;
	"cpu_session")
	cpun=`cat /proc/cpuinfo | grep -c processor`;
	case $cpun in
		"1")
		sqlite -cpu /tmp/db_cpu.db "select timeEnter,value from c_info where type=4 and timeEnter >strftime('%s','now','-1 hour') order by timeEnter asc" > /tmp/QRTG/cpu11.inf
		sqlite -cpu /tmp/db_cpu.db "select timeEnter,value from c_info2 where type=4 and timeEnter >strftime('%s','now','-1 day') order by timeEnter asc" > /tmp/QRTG/cpu21.inf
		sqlite -cpu /tmp/db_cpu.db "select timeEnter,value from c_info3 where type=4 and timeEnter >strftime('%s','now','-7 day') order by timeEnter asc" > /tmp/QRTG/cpu31.inf
		;;
		"2")
		sqlite -cpu /tmp/db_cpu.db "select timeEnter,value from c_info where type=4 and timeEnter >strftime('%s','now','-1 hour') order by timeEnter asc" > /tmp/QRTG/cpu11.inf
		sqlite -cpu /tmp/db_cpu.db "select timeEnter,value from c_info2 where type=4 and timeEnter >strftime('%s','now','-1 day') order by timeEnter asc" > /tmp/QRTG/cpu21.inf
		sqlite -cpu /tmp/db_cpu.db "select timeEnter,value from c_info3 where type=4 and timeEnter >strftime('%s','now','-7 day') order by timeEnter asc" > /tmp/QRTG/cpu31.inf
		sqlite -cpu /tmp/db_cpu.db "select timeEnter,value from c_info where type=5 and timeEnter >strftime('%s','now','-1 hour') order by timeEnter asc" > /tmp/QRTG/cpu12.inf
		sqlite -cpu /tmp/db_cpu.db "select timeEnter,value from c_info2 where type=5 and timeEnter >strftime('%s','now','-1 day') order by timeEnter asc" > /tmp/QRTG/cpu22.inf
		sqlite -cpu /tmp/db_cpu.db "select timeEnter,value from c_info3 where type=5 and timeEnter >strftime('%s','now','-7 day') order by timeEnter asc" > /tmp/QRTG/cpu32.inf
		;;
		"4")
		sqlite -cpu /tmp/db_cpu.db "select timeEnter,value from c_info where type=4 and timeEnter >strftime('%s','now','-1 hour') order by timeEnter asc" > /tmp/QRTG/cpu11.inf
		sqlite -cpu /tmp/db_cpu.db "select timeEnter,value from c_info2 where type=4 and timeEnter >strftime('%s','now','-1 day') order by timeEnter asc" > /tmp/QRTG/cpu21.inf
		sqlite -cpu /tmp/db_cpu.db "select timeEnter,value from c_info3 where type=4 and timeEnter >strftime('%s','now','-7 day') order by timeEnter asc" > /tmp/QRTG/cpu31.inf
		sqlite -cpu /tmp/db_cpu.db "select timeEnter,value from c_info where type=5 and timeEnter >strftime('%s','now','-1 hour') order by timeEnter asc" > /tmp/QRTG/cpu12.inf
		sqlite -cpu /tmp/db_cpu.db "select timeEnter,value from c_info2 where type=5 and timeEnter >strftime('%s','now','-1 day') order by timeEnter asc" > /tmp/QRTG/cpu22.inf
		sqlite -cpu /tmp/db_cpu.db "select timeEnter,value from c_info3 where type=5 and timeEnter >strftime('%s','now','-7 day') order by timeEnter asc" > /tmp/QRTG/cpu32.inf
		sqlite -cpu /tmp/db_cpu.db "select timeEnter,value from c_info where type=6 and timeEnter >strftime('%s','now','-1 hour') order by timeEnter asc" > /tmp/QRTG/cpu13.inf
		sqlite -cpu /tmp/db_cpu.db "select timeEnter,value from c_info2 where type=6 and timeEnter >strftime('%s','now','-1 day') order by timeEnter asc" > /tmp/QRTG/cpu23.inf
		sqlite -cpu /tmp/db_cpu.db "select timeEnter,value from c_info3 where type=6 and timeEnter >strftime('%s','now','-7 day') order by timeEnter asc" > /tmp/QRTG/cpu33.inf
		sqlite -cpu /tmp/db_cpu.db "select timeEnter,value from c_info where type=7 and timeEnter >strftime('%s','now','-1 hour') order by timeEnter asc" > /tmp/QRTG/cpu14.inf
		sqlite -cpu /tmp/db_cpu.db "select timeEnter,value from c_info2 where type=7 and timeEnter >strftime('%s','now','-1 day') order by timeEnter asc" > /tmp/QRTG/cpu24.inf
		sqlite -cpu /tmp/db_cpu.db "select timeEnter,value from c_info3 where type=7 and timeEnter >strftime('%s','now','-7 day') order by timeEnter asc" > /tmp/QRTG/cpu34.inf
		;;
		[q,Q])
			exit 0
		;;
	esac

	sqlite -avg /tmp/db_cpu.db "select avg(value) from c_info where type=1 and timeEnter >strftime('%s','now','-1 hour')" > /tmp/QRTG/cpuaa.inf
	sqlite -avg /tmp/db_cpu.db "select avg(value) from c_info2 where type=1 and timeEnter >strftime('%s','now','-1 day')" >> /tmp/QRTG/cpuaa.inf
	sqlite -avg /tmp/db_cpu.db "select avg(value) from c_info3 where type=1 and timeEnter >strftime('%s','now','-7 day')" >> /tmp/QRTG/cpuaa.inf
	sqlite -avg /tmp/db_cpu.db "select value from c_info where type=2 and timeEnter >strftime('%s','now','-1 hour') order by timeEnter desc limit 1" >> /tmp/QRTG/cpuaa.inf
	sqlite -avg /tmp/db_cpu.db "select value from c_info where type=3 and timeEnter >strftime('%s','now','-1 hour') order by timeEnter desc limit 1" >> /tmp/QRTG/cpuaa.inf

	sqlite -cpu /tmp/db_cpu.db "select timeEnter,value from c_info where type=1 and timeEnter >strftime('%s','now','-1 hour') order by timeEnter asc" > /tmp/QRTG/cpu1.inf
	sqlite -cpu /tmp/db_cpu.db "select timeEnter,value from c_info2 where type=1 and timeEnter >strftime('%s','now','-1 day') order by timeEnter asc" > /tmp/QRTG/cpu2.inf
	sqlite -cpu /tmp/db_cpu.db "select timeEnter,value from c_info3 where type=1 and timeEnter >strftime('%s','now','-7 day') order by timeEnter asc" > /tmp/QRTG/cpu3.inf

	sqlite -session /tmp/db_cpu.db "select timeEnter,value from c_info where type=3 and timeEnter >strftime('%s','now','-1 hour') order by timeEnter asc" > /tmp/QRTG/session1.inf
	sqlite -session /tmp/db_cpu.db "select timeEnter,value from c_info2 where type=3 and timeEnter >strftime('%s','now','-1 day') order by timeEnter asc" > /tmp/QRTG/session2.inf
	sqlite -session /tmp/db_cpu.db "select timeEnter,value from c_info3 where type=3 and timeEnter >strftime('%s','now','-7 day') order by timeEnter asc" > /tmp/QRTG/session3.inf

	sqlite -avg /tmp/db_cpu.db "select avg(value) from c_info where type=3 and timeEnter >strftime('%s','now','-1 hour')" > /tmp/QRTG/sessionaa.inf
	sqlite -avg /tmp/db_cpu.db "select avg(value) from c_info2 where type=3 and timeEnter >strftime('%s','now','-1 day')" >> /tmp/QRTG/sessionaa.inf
	sqlite -avg /tmp/db_cpu.db "select avg(value) from c_info3 where type=3 and timeEnter >strftime('%s','now','-7 day')" >> /tmp/QRTG/sessionaa.inf

	sqlite -avg /tmp/db_cpu.db "select max(value) from c_info where type=3 and timeEnter >strftime('%s','now','-1 hour')" > /tmp/QRTG/sessionam.inf
	sqlite -avg /tmp/db_cpu.db "select max(value) from c_info2 where type=3 and timeEnter >strftime('%s','now','-1 day')" >> /tmp/QRTG/sessionam.inf
	sqlite -avg /tmp/db_cpu.db "select max(value) from c_info where type=3 and timeEnter >strftime('%s','now','-7 day')" >> /tmp/QRTG/sessionam.inf
	;;
	"cpu_session_avg")
	sqlite -avg /tmp/db_cpu.db "select avg(value) from c_info where type=1 and timeEnter >strftime('%s','now','-1 hour')" > /tmp/QRTG/cpuaa.inf
	sqlite -avg /tmp/db_cpu.db "select avg(value) from c_info2 where type=1 and timeEnter >strftime('%s','now','-1 day')" >> /tmp/QRTG/cpuaa.inf
	sqlite -avg /tmp/db_cpu.db "select avg(value) from c_info3 where type=1 and timeEnter >strftime('%s','now','-7 day')" >> /tmp/QRTG/cpuaa.inf
	sqlite -avg /tmp/db_cpu.db "select value from c_info where type=2 and timeEnter >strftime('%s','now','-1 hour') order by timeEnter desc limit 1" >> /tmp/QRTG/cpuaa.inf

	sqlite -avg /tmp/db_cpu.db "select avg(value) from c_info where type=3 and timeEnter >strftime('%s','now','-1 hour')" > /tmp/QRTG/sessionaa.inf
	sqlite -avg /tmp/db_cpu.db "select avg(value) from c_info2 where type=3 and timeEnter >strftime('%s','now','-1 day')" >> /tmp/QRTG/sessionaa.inf
	sqlite -avg /tmp/db_cpu.db "select avg(value) from c_info3 where type=3 and timeEnter >strftime('%s','now','-7 day')" >> /tmp/QRTG/sessionaa.inf

	sqlite -avg /tmp/db_cpu.db "select max(value) from c_info where type=1 and timeEnter >strftime('%s','now','-1 hour')" > /tmp/QRTG/cpuam.inf
	sqlite -avg /tmp/db_cpu.db "select max(value) from c_info2 where type=1 and timeEnter >strftime('%s','now','-1 day')" >> /tmp/QRTG/cpuam.inf
	sqlite -avg /tmp/db_cpu.db "select max(value) from c_info3 where type=1 and timeEnter >strftime('%s','now','-7 day')" >> /tmp/QRTG/cpuam.inf

	sqlite -avg /tmp/db_cpu.db "select max(value) from c_info where type=3 and timeEnter >strftime('%s','now','-1 hour')" > /tmp/QRTG/sessionam.inf
	sqlite -avg /tmp/db_cpu.db "select max(value) from c_info2 where type=3 and timeEnter >strftime('%s','now','-1 day')" >> /tmp/QRTG/sessionam.inf
	sqlite -avg /tmp/db_cpu.db "select max(value) from c_info where type=3 and timeEnter >strftime('%s','now','-7 day')" >> /tmp/QRTG/sessionam.inf
	;;
	"home.htm")
	if [ -e /tmp/QRTG/cpumem.inf ]; then
		rm /tmp/QRTG/cpumem.inf;
	fi
	cpucore=`cat /proc/cpuinfo | grep -c processor`;
	while [ "$cpucore" -gt "0" ] 
	do
		printf `sqlite -avg /tmp/db_cpu.db "select value from c_info \
		where type=1 order by timeEnter desc limit 1"` >> /tmp/QRTG/cpumem.inf;
		cpucore=$(($cpucore-1));
	done
	sqlite -avg /tmp/db_cpu.db "select value from c_info where type=2 order by timeEnter desc limit 1" >> /tmp/QRTG/cpumem.inf
	sqlite -avg /tmp/db_cpu.db "select value from c_info where type=3 order by timeEnter desc limit 1" >> /tmp/QRTG/cpumem.inf
	;;
	[q,Q])
		exit 0
	;;
	*)
		exit 0
	;;
esac
