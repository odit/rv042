
#!/bin/bash
# 
# read -p "test:" input_txt
case $1 in
   	"0")
		sqlite /tmp/db.db "create table t2(type int,value int,timeEnter DATE)"
		sqlite /tmp/db.db "create table t3(type int,value int,timeEnter DATE)"
		sqlite /tmp/db.db "create table t4(type int,value int,timeEnter DATE)"

		sqlite /tmp/db.db "create index t2_i on t2(type int,value int,timeEnter DATE)"
		sqlite /tmp/db.db "create index t3_i on t3(type int,value int,timeEnter DATE)"
		sqlite /tmp/db.db "create index t4_i on t4(type int,value int,timeEnter DATE)"
		
		sqlite /tmp/db_cpu.db "create table c_info(type int,value int,timeEnter DATE)"
		sqlite /tmp/db_cpu.db "create table c_info2(type int,value int,timeEnter DATE)"
		sqlite /tmp/db_cpu.db "create table c_info3(type int,value int,timeEnter DATE)"

		sqlite /tmp/db_cpu.db "create index c1_i on c_info(type int,value int,timeEnter DATE)"
		sqlite /tmp/db_cpu.db "create index c2_i on c_info2(type int,value int,timeEnter DATE)"
		sqlite /tmp/db_cpu.db "create index c3_i on c_info3(type int,value int,timeEnter DATE)"

		sqlite /tmp/db_cpu.db "insert into c_info values (1,0,STRFTIME('%s','NOW'))"
		sqlite /tmp/db_cpu.db "insert into c_info values (2,0,STRFTIME('%s','NOW'));"
		sqlite /tmp/db_cpu.db "insert into c_info values (3,0,STRFTIME('%s','NOW'));"
		sqlite /tmp/db_cpu.db "insert into c_info values (4,0,STRFTIME('%s','NOW'));"
		sqlite /tmp/db_cpu.db "insert into c_info values (5,0,STRFTIME('%s','NOW'));"

		sqlite /tmp/db_cpu.db "insert into c_info2 values (1,0,STRFTIME('%s','NOW'))"
		sqlite /tmp/db_cpu.db "insert into c_info2 values (2,0,STRFTIME('%s','NOW'));"
		sqlite /tmp/db_cpu.db "insert into c_info2 values (3,0,STRFTIME('%s','NOW'));"
		sqlite /tmp/db_cpu.db "insert into c_info2 values (4,0,STRFTIME('%s','NOW'));"
		sqlite /tmp/db_cpu.db "insert into c_info2 values (5,0,STRFTIME('%s','NOW'));"

		sqlite /tmp/db_cpu.db "insert into c_info3 values (1,0,STRFTIME('%s','NOW'))"
		sqlite /tmp/db_cpu.db "insert into c_info3 values (2,0,STRFTIME('%s','NOW'));"
		sqlite /tmp/db_cpu.db "insert into c_info3 values (3,0,STRFTIME('%s','NOW'));"
		sqlite /tmp/db_cpu.db "insert into c_info3 values (4,0,STRFTIME('%s','NOW'));"
		sqlite /tmp/db_cpu.db "insert into c_info3 values (5,0,STRFTIME('%s','NOW'));"

		sqlite /tmp/db.db "BEGIN;
			insert into t2 values (4,0,STRFTIME('%s','NOW'));
			insert into t2 values (5,0,STRFTIME('%s','NOW'));
			insert into t2 values (6,0,STRFTIME('%s','NOW'));
			insert into t2 values (7,0,STRFTIME('%s','NOW'));
			insert into t2 values (8,0,STRFTIME('%s','NOW'));
			insert into t2 values (9,0,STRFTIME('%s','NOW'));
			insert into t2 values (10,0,STRFTIME('%s','NOW'));
			insert into t2 values (11,0,STRFTIME('%s','NOW'));
			insert into t2 values (12,0,STRFTIME('%s','NOW'));
			insert into t2 values (13,0,STRFTIME('%s','NOW'));
			insert into t2 values (14,0,STRFTIME('%s','NOW'));
			insert into t2 values (15,0,STRFTIME('%s','NOW'));
			insert into t2 values (16,0,STRFTIME('%s','NOW'));
			insert into t2 values (17,0,STRFTIME('%s','NOW'));
			insert into t2 values (18,0,STRFTIME('%s','NOW'));
			insert into t2 values (19,0,STRFTIME('%s','NOW'));
			insert into t2 values (20,0,STRFTIME('%s','NOW'));
			insert into t2 values (21,0,STRFTIME('%s','NOW'));
			COMMIT;"
	;;
	"1.0")
		ck_table=`sqlite /tmp/db.db ".table" | awk '{print $1$2$3}'`
		if [ "$ck_table" = "t2t3t4" ]; then
			if [ "$2" = "4" ]; then
				meminfo=`sar -r 1 1 | grep Average | awk '{print $4}'`
				sessioninfo=`cat /proc/slabinfo | grep ip_conntrack\  | awk {'print $2'}`
				case "$14" in
   				"1")
					sqlite /tmp/db_cpu.db "BEGIN;
					insert into c_info values (1,$3,STRFTIME('%s','NOW'));
					insert into c_info values (2,$meminfo,STRFTIME('%s','NOW'));
					insert into c_info values (3,$sessioninfo,STRFTIME('%s','NOW'));
					insert into c_info values (4,$17,STRFTIME('%s','NOW'));
					COMMIT;"
				;;
				"2")
					sqlite /tmp/db_cpu.db "BEGIN;
					insert into c_info values (1,$3,STRFTIME('%s','NOW'));
					insert into c_info values (2,$meminfo,STRFTIME('%s','NOW'));
					insert into c_info values (3,$sessioninfo,STRFTIME('%s','NOW'));
					insert into c_info values (4,$17,STRFTIME('%s','NOW'));
					insert into c_info values (5,$18,STRFTIME('%s','NOW'));
					COMMIT;"
				;;
				"4")
					sqlite /tmp/db_cpu.db "BEGIN;
					insert into c_info values (1,$3,STRFTIME('%s','NOW'));
					insert into c_info values (2,$meminfo,STRFTIME('%s','NOW'));
					insert into c_info values (3,$sessioninfo,STRFTIME('%s','NOW'));
					insert into c_info values (4,$17,STRFTIME('%s','NOW'));
					insert into c_info values (5,$18,STRFTIME('%s','NOW'));
					insert into c_info values (6,$19,STRFTIME('%s','NOW'));
					insert into c_info values (7,$20,STRFTIME('%s','NOW'));
					COMMIT;"
				;;
				"8")
					sqlite /tmp/db_cpu.db "BEGIN;
					insert into c_info values (1,$3,STRFTIME('%s','NOW'));
					insert into c_info values (2,$meminfo,STRFTIME('%s','NOW'));
					insert into c_info values (3,$sessioninfo,STRFTIME('%s','NOW'));
					insert into c_info values (4,$17,STRFTIME('%s','NOW'));
					insert into c_info values (5,$18,STRFTIME('%s','NOW'));
					insert into c_info values (6,$19,STRFTIME('%s','NOW'));
					insert into c_info values (7,$20,STRFTIME('%s','NOW'));
					insert into c_info values (8,$21,STRFTIME('%s','NOW'));
					insert into c_info values (9,$22,STRFTIME('%s','NOW'));
					insert into c_info values (10,$23,STRFTIME('%s','NOW'));
					insert into c_info values (11,$24,STRFTIME('%s','NOW'));
					COMMIT;"
				;;
				esac

				sqlite /tmp/db.db "BEGIN;
				insert into t2 values (4,$6,STRFTIME('%s','NOW'));
				insert into t2 values (5,$8,STRFTIME('%s','NOW'));
				insert into t2 values (6,$10,STRFTIME('%s','NOW'));
				insert into t2 values (7,$12,STRFTIME('%s','NOW'));
				insert into t2 values (9,$7,STRFTIME('%s','NOW'));
				insert into t2 values (10,$9,STRFTIME('%s','NOW'));
				insert into t2 values (11,$11,STRFTIME('%s','NOW'));
				insert into t2 values (12,$13,STRFTIME('%s','NOW'));
				COMMIT;"
			fi
			if [ "$2" -le "5" ]; then
				meminfo=`sar -r 1 1 | grep Average | awk '{print $4}'`
				sessioninfo=`cat /proc/slabinfo | grep ip_conntrack\  | awk {'print $2'}`
				case "$17" in
   				"1")
					sqlite /tmp/db_cpu.db "BEGIN;
					insert into c_info values (1,$3,STRFTIME('%s','NOW'));
					insert into c_info values (2,$meminfo,STRFTIME('%s','NOW'));
					insert into c_info values (3,$sessioninfo,STRFTIME('%s','NOW'));
					insert into c_info values (4,$20,STRFTIME('%s','NOW'));
					COMMIT;"
				;;
				"2")
					sqlite /tmp/db_cpu.db "BEGIN;
					insert into c_info values (1,$3,STRFTIME('%s','NOW'));
					insert into c_info values (2,$meminfo,STRFTIME('%s','NOW'));
					insert into c_info values (3,$sessioninfo,STRFTIME('%s','NOW'));
					insert into c_info values (4,$20,STRFTIME('%s','NOW'));
					insert into c_info values (5,$21,STRFTIME('%s','NOW'));
					COMMIT;"
				;;
				"4")
					sqlite /tmp/db_cpu.db "BEGIN;
					insert into c_info values (1,$3,STRFTIME('%s','NOW'));
					insert into c_info values (2,$meminfo,STRFTIME('%s','NOW'));
					insert into c_info values (3,$sessioninfo,STRFTIME('%s','NOW'));
					insert into c_info values (4,$20,STRFTIME('%s','NOW'));
					insert into c_info values (5,$21,STRFTIME('%s','NOW'));
					insert into c_info values (6,$22,STRFTIME('%s','NOW'));
					insert into c_info values (7,$23,STRFTIME('%s','NOW'));
					COMMIT;"
				;;
				"8")
					sqlite /tmp/db_cpu.db "BEGIN;
					insert into c_info values (1,$3,STRFTIME('%s','NOW'));
					insert into c_info values (2,$meminfo,STRFTIME('%s','NOW'));
					insert into c_info values (3,$sessioninfo,STRFTIME('%s','NOW'));
					insert into c_info values (4,$20,STRFTIME('%s','NOW'));
					insert into c_info values (5,$21,STRFTIME('%s','NOW'));
					insert into c_info values (6,$22,STRFTIME('%s','NOW'));
					insert into c_info values (7,$23,STRFTIME('%s','NOW'));
					insert into c_info values (8,$24,STRFTIME('%s','NOW'));
					insert into c_info values (9,$25,STRFTIME('%s','NOW'));
					insert into c_info values (10,$26,STRFTIME('%s','NOW'));
					insert into c_info values (11,$27,STRFTIME('%s','NOW'));
					COMMIT;"
				;;
				esac

				sqlite /tmp/db.db "BEGIN;
				insert into t2 values (4,$6,STRFTIME('%s','NOW'));
				insert into t2 values (5,$8,STRFTIME('%s','NOW'));
				insert into t2 values (6,$10,STRFTIME('%s','NOW'));
				insert into t2 values (7,$12,STRFTIME('%s','NOW'));
				insert into t2 values (8,$14,STRFTIME('%s','NOW'));
				insert into t2 values (9,$7,STRFTIME('%s','NOW'));
				insert into t2 values (10,$9,STRFTIME('%s','NOW'));
				insert into t2 values (11,$11,STRFTIME('%s','NOW'));
				insert into t2 values (12,$13,STRFTIME('%s','NOW'));
				insert into t2 values (13,$15,STRFTIME('%s','NOW'));
				COMMIT;"
			fi
			if [ "$2" = "8" ]; then
				meminfo=`sar -r 1 1 | grep Average | awk '{print $4}'`
				sessioninfo=`cat /proc/slabinfo | grep ip_conntrack\  | awk {'print $2'}`
				case "$23" in
   				"1")
					sqlite /tmp/db_cpu.db "BEGIN;
					insert into c_info values (1,$3,STRFTIME('%s','NOW'));
					insert into c_info values (2,$meminfo,STRFTIME('%s','NOW'));
					insert into c_info values (3,$sessioninfo,STRFTIME('%s','NOW'));
					insert into c_info values (4,$26,STRFTIME('%s','NOW'));
					COMMIT;"
				;;
				"2")
					sqlite /tmp/db_cpu.db "BEGIN;
					insert into c_info values (1,$3,STRFTIME('%s','NOW'));
					insert into c_info values (2,$meminfo,STRFTIME('%s','NOW'));
					insert into c_info values (3,$sessioninfo,STRFTIME('%s','NOW'));
					insert into c_info values (4,$26,STRFTIME('%s','NOW'));
					insert into c_info values (5,$27,STRFTIME('%s','NOW'));
					COMMIT;"
				;;
				"4")
					sqlite /tmp/db_cpu.db "BEGIN;
					insert into c_info values (1,$3,STRFTIME('%s','NOW'));
					insert into c_info values (2,$meminfo,STRFTIME('%s','NOW'));
					insert into c_info values (3,$sessioninfo,STRFTIME('%s','NOW'));
					insert into c_info values (4,$26,STRFTIME('%s','NOW'));
					insert into c_info values (5,$27,STRFTIME('%s','NOW'));
					insert into c_info values (6,$28,STRFTIME('%s','NOW'));
					insert into c_info values (7,$29,STRFTIME('%s','NOW'));
					COMMIT;"
				;;
				"8")
					sqlite /tmp/db_cpu.db "BEGIN;
					insert into c_info values (1,$3,STRFTIME('%s','NOW'));
					insert into c_info values (2,$meminfo,STRFTIME('%s','NOW'));
					insert into c_info values (3,$sessioninfo,STRFTIME('%s','NOW'));
					insert into c_info values (4,$26,STRFTIME('%s','NOW'));
					insert into c_info values (5,$27,STRFTIME('%s','NOW'));
					insert into c_info values (6,$28,STRFTIME('%s','NOW'));
					insert into c_info values (7,$29,STRFTIME('%s','NOW'));
					insert into c_info values (8,$30,STRFTIME('%s','NOW'));
					insert into c_info values (9,$31,STRFTIME('%s','NOW'));
					insert into c_info values (10,$32,STRFTIME('%s','NOW'));
					insert into c_info values (11,$33,STRFTIME('%s','NOW'));
					COMMIT;"
				;;
				esac

				sqlite /tmp/db.db "BEGIN;
				insert into t2 values (4,$6,STRFTIME('%s','NOW'));
				insert into t2 values (5,$8,STRFTIME('%s','NOW'));
				insert into t2 values (6,$10,STRFTIME('%s','NOW'));
				insert into t2 values (7,$12,STRFTIME('%s','NOW'));
				insert into t2 values (8,$14,STRFTIME('%s','NOW'));
				insert into t2 values (9,$16,STRFTIME('%s','NOW'));
				insert into t2 values (10,$18,STRFTIME('%s','NOW'));
				insert into t2 values (11,$20,STRFTIME('%s','NOW'));
				insert into t2 values (12,$7,STRFTIME('%s','NOW'));
				insert into t2 values (13,$9,STRFTIME('%s','NOW'));
				insert into t2 values (14,$11,STRFTIME('%s','NOW'));
				insert into t2 values (15,$13,STRFTIME('%s','NOW'));
				insert into t2 values (16,$15,STRFTIME('%s','NOW'));
				insert into t2 values (17,$17,STRFTIME('%s','NOW'));
				insert into t2 values (18,$19,STRFTIME('%s','NOW'));
				insert into t2 values (19,$21,STRFTIME('%s','NOW'));
				COMMIT;"
			fi
		else
			sqlite /tmp/db.db "create table t2(type int,value int,timeEnter DATE)"
			sqlite /tmp/db.db "create table t3(type int,value int,timeEnter DATE)"
			sqlite /tmp/db.db "create table t4(type int,value int,timeEnter DATE)"
			
			sqlite /tmp/db_cpu.db "create table c_info(type int,value int,timeEnter DATE)"
			sqlite /tmp/db_cpu.db "create table c_info2(type int,value int,timeEnter DATE)"
			sqlite /tmp/db_cpu.db "create table c_info3(type int,value int,timeEnter DATE)"
		fi	

	;;
	"2")
		case "$3" in
			"1")
			for item in "1" "2" "3" "4" ; do 
			sqlite /tmp/db_cpu.db "insert into c_info2 (type,value,timeEnter) select avg(type),avg(value),STRFTIME('%s','NOW') from c_info where type=$item order by timeEnter desc limit 20"
			done
			;;
			"2")
			for item in "1" "2" "3" "4" "5" ; do 
			sqlite /tmp/db_cpu.db "insert into c_info2 (type,value,timeEnter) select avg(type),avg(value),STRFTIME('%s','NOW') from c_info where type=$item order by timeEnter desc limit 20"
			done
			;;
			"4")
			for item in "1" "2" "3" "4" "5" "6" "7" ; do 
			sqlite /tmp/db_cpu.db "insert into c_info2 (type,value,timeEnter) select avg(type),avg(value),STRFTIME('%s','NOW') from c_info where type=$item order by timeEnter desc limit 20"
			done
			;;
			"8")
			for item in "1" "2" "3" "4" "5" "6" "7" "8" "9" "10" "11" ; do 
			sqlite /tmp/db_cpu.db "insert into c_info2 (type,value,timeEnter) select avg(type),avg(value),STRFTIME('%s','NOW') from c_info where type=$item order by timeEnter desc limit 20"
			done
		;;
		esac
		if [ "$2" -le "5" ]; then
			for item in "4" "5" "6" "7" "8" "9" "10" "11" "12" "13" ; do 
			sqlite /tmp/db.db "insert into t3 (type,value,timeEnter) select avg(type),avg(value),STRFTIME('%s','NOW') from t2 where type=$item order by timeEnter desc limit 20"
			done

			db_sum_h=`sqlite /tmp/db.db "select count(*) from t2"`
			db_sum_d=`sqlite /tmp/db.db "select count(*) from t3"`
			db_sum_w=`sqlite /tmp/db.db "select count(*) from t4"`
			if [ "$db_sum_h" -ge "3360" ]; then
				sqlite /tmp/db_cpu.db "delete from c_info where timeEnter <STRFTIME('%s','NOW','-2 hour')"
				sqlite /tmp/db.db "delete from t2 where timeEnter <STRFTIME('%s','NOW','-2 hour')"
			fi
			if [ "$db_sum_d" -ge "4032" ]; then
				sqlite /tmp/db_cpu.db "delete from c_info2 where timeEnter <STRFTIME('%s','NOW','-2 day')"
				sqlite /tmp/db.db "delete from t3 where timeEnter <STRFTIME('%s','NOW','-2 day')"
			fi
			if [ "$db_sum_w" -ge "4704" ]; then
				sqlite /tmp/db_cpu.db "delete from c_info3 where timeEnter <STRFTIME('%s','NOW','-14 day')"
				sqlite /tmp/db.db "delete from t4 where timeEnter <STRFTIME('%s','NOW','-14 day')"
			fi

		fi
		if [ "$2" = "8" ]; then
			for item in "4" "5" "6" "7" "8" "9" "10" "11" "12" "13" "14" "15" "16" "17" "18" "19" "20" "21" ; do 
			sqlite /tmp/db.db "insert into t3 (type,value,timeEnter) select avg(type),avg(value),STRFTIME('%s','NOW') from t2 where type=$item order by timeEnter desc limit 20"
			done

			db_sum_h=`sqlite /tmp/db.db "select count(*) from t2"`
			db_sum_d=`sqlite /tmp/db.db "select count(*) from t3"`
			db_sum_w=`sqlite /tmp/db.db "select count(*) from t4"`
			if [ "$db_sum_h" -ge "4800" ]; then
				sqlite /tmp/db_cpu.db "delete from c_info where timeEnter <STRFTIME('%s','NOW','-2 hour')"
				sqlite /tmp/db.db "delete from t2 where timeEnter <STRFTIME('%s','NOW','-2 hour')"
			fi
			if [ "$db_sum_d" -ge "5760" ]; then
				sqlite /tmp/db_cpu.db "delete from c_info2 where timeEnter <STRFTIME('%s','NOW','-2 day')"
				sqlite /tmp/db.db "delete from t3 where timeEnter <STRFTIME('%s','NOW','-2 day')"
			fi
			if [ "$db_sum_w" -ge "6720" ]; then
				sqlite /tmp/db_cpu.db "delete from c_info3 where timeEnter <STRFTIME('%s','NOW','-14 day')"
				sqlite /tmp/db.db "delete from t4 where timeEnter <STRFTIME('%s','NOW','-14 day')"
			fi

		fi
	;;
	"3")
		case "$3" in
			"1")
			for item in "1" "2" "3" "4" ; do 
			sqlite /tmp/db_cpu.db "insert into c_info3 (type,value,timeEnter) select avg(type),avg(value),STRFTIME('%s','NOW') from c_info2 where type=$item order by timeEnter desc limit 6"
			done
			;;
			"2")
			for item in "1" "2" "3" "4" "5" ; do 
			sqlite /tmp/db_cpu.db "insert into c_info3 (type,value,timeEnter) select avg(type),avg(value),STRFTIME('%s','NOW') from c_info2 where type=$item order by timeEnter desc limit 6"
			done
			;;
			"4")
			for item in "1" "2" "3" "4" "5" "6" "7" ; do 
			sqlite /tmp/db_cpu.db "insert into c_info3 (type,value,timeEnter) select avg(type),avg(value),STRFTIME('%s','NOW') from c_info2 where type=$item order by timeEnter desc limit 6"
			done
			;;
			"8")
			for item in "1" "2" "3" "4" "5" "6" "7" "8" "9" "10" "11" ; do 
			sqlite /tmp/db_cpu.db "insert into c_info3 (type,value,timeEnter) select avg(type),avg(value),STRFTIME('%s','NOW') from c_info2 where type=$item order by timeEnter desc limit 6"
			done
		;;
		esac
		if [ "$2" -le "5" ]; then
			for item2 in "4" "5" "6" "7" "8" "9" "10" "11" "12" "13" ; do 
			sqlite /tmp/db.db "insert into t4 (type,value,timeEnter) select avg(type),avg(value),STRFTIME('%s','NOW') from t2 where type=$item2 order by timeEnter desc limit 6"
			done
		fi
		if [ "$2" = "8" ]; then
			for item2 in "4" "5" "6" "7" "8" "9" "10" "11" "12" "13" "14" "15" "16" "17" "18" "19" "20" "21" ; do 
			sqlite /tmp/db.db "insert into t4 (type,value,timeEnter) select avg(type),avg(value),STRFTIME('%s','NOW') from t2 where type=$item2 order by timeEnter desc limit 6"
			done
		fi
	;;
   	"4")
# 		sleep 30
		sqlite /tmp/db_cpu.db "insert into c_info values (1,0,STRFTIME('%s','NOW'))"
		sqlite /tmp/db_cpu.db "insert into c_info values (2,0,STRFTIME('%s','NOW'))"
		sqlite /tmp/db_cpu.db "insert into c_info values (3,0,STRFTIME('%s','NOW'))"
		sqlite /tmp/db_cpu.db "insert into c_info values (4,0,STRFTIME('%s','NOW'))"
		sqlite /tmp/db_cpu.db "insert into c_info values (5,0,STRFTIME('%s','NOW'))"

		sqlite /tmp/db_cpu.db "insert into c_info2 values (1,0,STRFTIME('%s','NOW'))";
		sqlite /tmp/db_cpu.db "insert into c_info2 values (2,0,STRFTIME('%s','NOW'));"
		sqlite /tmp/db_cpu.db "insert into c_info2 values (3,0,STRFTIME('%s','NOW'));"
		sqlite /tmp/db_cpu.db "insert into c_info2 values (4,0,STRFTIME('%s','NOW'));"
		sqlite /tmp/db_cpu.db "insert into c_info2 values (5,0,STRFTIME('%s','NOW'));"

		sqlite /tmp/db_cpu.db "insert into c_info3 values (1,0,STRFTIME('%s','NOW'))"
		sqlite /tmp/db_cpu.db "insert into c_info3 values (2,0,STRFTIME('%s','NOW'));"
		sqlite /tmp/db_cpu.db "insert into c_info3 values (3,0,STRFTIME('%s','NOW'));"
		sqlite /tmp/db_cpu.db "insert into c_info3 values (4,0,STRFTIME('%s','NOW'));"
		sqlite /tmp/db_cpu.db "insert into c_info3 values (5,0,STRFTIME('%s','NOW'));"
		sqlite /tmp/db.db "BEGIN;
			insert into t2 values (2,0,STRFTIME('%s','NOW'));
			insert into t2 values (3,0,STRFTIME('%s','NOW'));
			insert into t2 values (4,0,STRFTIME('%s','NOW'));
			insert into t2 values (5,0,STRFTIME('%s','NOW'));
			insert into t2 values (6,0,STRFTIME('%s','NOW'));
			insert into t2 values (7,0,STRFTIME('%s','NOW'));
			insert into t2 values (8,0,STRFTIME('%s','NOW'));
			insert into t2 values (9,0,STRFTIME('%s','NOW'));
			insert into t2 values (10,0,STRFTIME('%s','NOW'));
			insert into t2 values (11,0,STRFTIME('%s','NOW'));
			insert into t2 values (12,0,STRFTIME('%s','NOW'));
			insert into t2 values (13,0,STRFTIME('%s','NOW'));
			insert into t2 values (14,0,STRFTIME('%s','NOW'));
			insert into t2 values (15,0,STRFTIME('%s','NOW'));
			insert into t2 values (16,0,STRFTIME('%s','NOW'));
			insert into t2 values (17,0,STRFTIME('%s','NOW'));
			insert into t2 values (18,0,STRFTIME('%s','NOW'));
			insert into t2 values (19,0,STRFTIME('%s','NOW'));
			insert into t2 values (20,0,STRFTIME('%s','NOW'));
			insert into t2 values (21,0,STRFTIME('%s','NOW'));
			COMMIT;"
	;;
    "d_hour")
		sqlite /tmp/db_cpu.db "delete from c_info where timeEnter <STRFTIME('%s','NOW','-2 hour')"
		sqlite /tmp/db.db "delete from t2 where timeEnter <STRFTIME('%s','NOW','-2 hour')"
	;;
    "d_day")
		sqlite /tmp/db_cpu.db "delete from c_info2 where timeEnter <STRFTIME('%s','NOW','-2 day')"
		sqlite /tmp/db.db "delete from t3 where timeEnter <STRFTIME('%s','NOW','-2 day')"
	;;
    "d_week")
		sqlite /tmp/db_cpu.db "delete from c_info3 where timeEnter <STRFTIME('%s','NOW','-14 day')"
		sqlite /tmp/db.db "delete from t4 where timeEnter <STRFTIME('%s','NOW','-14 day')"
	;;
    [q,Q])
	exit 0
	;;
    *)
	exit 0
	;;
esac
