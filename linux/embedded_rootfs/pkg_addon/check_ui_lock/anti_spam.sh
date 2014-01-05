
#!/bin/bash
# 

case "$1"
  in
    1)
	sqlite /tmp/db_anti.db "create table anti_ips(name char,ips_counter int,total_counter int)"
	sqlite /tmp/db_anti.db "insert into anti_ips values('anti_ips',0,0)"

	sqlite /tmp/db_anti.db "create table anti_ips_m(date int,ips_counter int,total_counter int)"
	sqlite /tmp/db_anti.db "create table anti_ips_h(date int,ips_counter int,total_counter int)"
	sqlite /tmp/db_anti.db "create table anti_ips_d(date int,ips_counter int,total_counter int)"
	sqlite /tmp/db_anti.db "create table anti_ips_t(date int,ips_counter int,total_counter int)"

	sqlite /tmp/db_anti.db "insert into anti_ips_m values ('$2',0,0)"
	sqlite /tmp/db_anti.db "insert into anti_ips_h values ('$2',0,0)"
	sqlite /tmp/db_anti.db "insert into anti_ips_d values ('$2',0,0)"
	sqlite /tmp/db_anti.db "insert into anti_ips_t values ('$2',0,0)"

	sqlite /tmp/db_anti.db "create table anti_virus(name char,virus_counter int,total_counter int)"
	sqlite /tmp/db_anti.db "insert into anti_virus values('anti_virus',0,0)"

	sqlite /tmp/db_anti.db "create table anti_virus_m(date int,virus_counter int,total_counter int)"
	sqlite /tmp/db_anti.db "create table anti_virus_h(date int,virus_counter int,total_counter int)"
	sqlite /tmp/db_anti.db "create table anti_virus_d(date int,virus_counter int,total_counter int)"
	sqlite /tmp/db_anti.db "create table anti_virus_t(date int,virus_counter int,total_counter int)"
	
	sqlite /tmp/db_anti.db "insert into anti_virus_m values ('$2',0,0)"
	sqlite /tmp/db_anti.db "insert into anti_virus_h values ('$2',0,0)"
	sqlite /tmp/db_anti.db "insert into anti_virus_d values ('$2',0,0)"
	sqlite /tmp/db_anti.db "insert into anti_virus_t values ('$2',0,0)"

	sqlite /tmp/db_anti.db "create table anti_spam(name char,spam_counter int,total_counter int)"
	sqlite /tmp/db_anti.db "insert into anti_spam values('anti_spam',0,0)"

	sqlite /tmp/db_anti.db "create table anti_spam_m(date int,spam_counter int,total_counter int)"
	sqlite /tmp/db_anti.db "create table anti_spam_h(date int,spam_counter int,total_counter int)"
	sqlite /tmp/db_anti.db "create table anti_spam_d(date int,spam_counter int,total_counter int)"
	sqlite /tmp/db_anti.db "create table anti_spam_t(date int,spam_counter int,total_counter int)"

	sqlite /tmp/db_anti.db "insert into anti_spam_m values ('$2',0,0)"
	sqlite /tmp/db_anti.db "insert into anti_spam_h values ('$2',0,0)"
	sqlite /tmp/db_anti.db "insert into anti_spam_d values ('$2',0,0)"
	sqlite /tmp/db_anti.db "insert into anti_spam_t values ('$2',0,0)"
	;;
    2)
	as_sum=`sqlite /tmp/db_anti.db "select ips_counter from anti_ips"`
	as_m_tt=`sqlite /tmp/db_anti.db "select total_counter from anti_ips"`
	sqlite /tmp/db_anti.db "insert into anti_ips_m values ('$2','$as_sum','$as_m_tt')"
	
	as_sum=`sqlite /tmp/db_anti.db "select virus_counter from anti_virus"`
	as_m_tt=`sqlite /tmp/db_anti.db "select total_counter from anti_virus"`
	sqlite /tmp/db_anti.db "insert into anti_virus_m values ('$2','$as_sum','$as_m_tt')"
		
	as_sum=`sqlite /tmp/db_anti.db "select spam_counter from anti_spam"`
	as_m_tt=`sqlite /tmp/db_anti.db "select total_counter from anti_spam"`
	sqlite /tmp/db_anti.db "insert into anti_spam_m values ('$2','$as_sum','$as_m_tt')"
	;;
    3)
	as_sum=`sqlite /tmp/db_anti.db "select sum(ips_counter) from anti_ips_m where date > strftime('%s','now','-1 hour') order by date desc limit 60"`
	as_h_tt=`sqlite /tmp/db_anti.db "select sum(total_counter) from anti_ips_m where date > strftime('%s','now','-1 hour') order by date desc limit 60"`
	sqlite /tmp/db_anti.db "insert into anti_ips_h values ('$2','$as_sum','$as_h_tt')"
		
	as_sum=`sqlite /tmp/db_anti.db "select sum(virus_counter) from anti_virus_m where date > strftime('%s','now','-1 hour') order by date desc limit 60"`
	as_h_tt=`sqlite /tmp/db_anti.db "select sum(total_counter) from anti_virus_m where date > strftime('%s','now','-1 hour') order by date desc limit 60"`
	sqlite /tmp/db_anti.db "insert into anti_virus_h values ('$2','$as_sum','$as_h_tt')"
	
	as_sum=`sqlite /tmp/db_anti.db "select sum(spam_counter) from anti_spam_m where date > strftime('%s','now','-1 hour') order by date desc limit 60"`
	as_h_tt=`sqlite /tmp/db_anti.db "select sum(total_counter) from anti_spam_m where date > strftime('%s','now','-1 hour') order by date desc limit 60"`
	sqlite /tmp/db_anti.db "insert into anti_spam_h values ('$2','$as_sum','$as_h_tt')"
	;;
    4)
	as_sum=`sqlite /tmp/db_anti.db "select sum(ips_counter) from anti_ips_h where date > strftime('%s','now','-1 day') order by date desc limit 24"`
	as_d_tt=`sqlite /tmp/db_anti.db "select sum(total_counter) from anti_ips_h where date > strftime('%s','now','-1 day') order by date desc limit 24"`
	sqlite /tmp/db_anti.db "insert into anti_ips_d values ('$2','$as_sum','$as_d_tt')"
	
	as_sum=`sqlite /tmp/db_anti.db "select sum(virus_counter) from anti_virus_h where date > strftime('%s','now','-1 day') order by date desc limit 24"`
	as_d_tt=`sqlite /tmp/db_anti.db "select sum(total_counter) from anti_virus_h where date > strftime('%s','now','-1 day') order by date desc limit 24"`
	sqlite /tmp/db_anti.db "insert into anti_virus_d values ('$2','$as_sum','$as_d_tt')"
	
	as_sum=`sqlite /tmp/db_anti.db "select sum(spam_counter) from anti_spam_h where date > strftime('%s','now','-1 day') order by date desc limit 24"`
	as_d_tt=`sqlite /tmp/db_anti.db "select sum(total_counter) from anti_spam_h where date > strftime('%s','now','-1 day') order by date desc limit 24"`
	sqlite /tmp/db_anti.db "insert into anti_spam_d values ('$2','$as_sum','$as_d_tt')"

	;;
    99)
	sqlite /tmp/db_anti.db "update anti_ips set ips_counter=0,total_counter=0 where name='anti_ips'"
	sqlite /tmp/db_anti.db "update anti_virus set virus_counter=0,total_counter=0 where name='anti_virus'"
	sqlite /tmp/db_anti.db "update anti_spam set spam_counter=0,total_counter=0 where name='anti_spam'"

	;;
    d_hour)
	sqlite /tmp/db_anti.db "delete from anti_ips_m where date < strftime('%s','now','-2 hour')"
	sqlite /tmp/db_anti.db "delete from anti_virus_m where date < strftime('%s','now','-2 hour')"
	sqlite /tmp/db_anti.db "delete from anti_spam_m where date < strftime('%s','now','-2 hour')"
	;;
    d_day)
	sqlite /tmp/db_anti.db "delete from anti_ips_h where date < strftime('%s','now','-2 day')"
	sqlite /tmp/db_anti.db "delete from anti_virus_h where date < strftime('%s','now','-2 day')"
	sqlite /tmp/db_anti.db "delete from anti_spam_h where date < strftime('%s','now','-2 day')"
	;;
    d_week)
	sqlite /tmp/db_anti.db "delete from anti_ips_d where date < strftime('%s','now','-14 day')"
	sqlite /tmp/db_anti.db "delete from anti_virus_d where date < strftime('%s','now','-14 day')"
	sqlite /tmp/db_anti.db "delete from anti_spam_d where date < strftime('%s','now','-14 day')"
	;;
    [q,Q])
	exit 0
	;;
    *)
	exit 0
	;;
esac
