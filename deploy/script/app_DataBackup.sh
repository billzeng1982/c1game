#!/bin/bash

. comm.sh

app=$0

if [ $# -lt 2 ]; then 
    echo "usage: $0 <start|stop|restart> <r|d>";
	exit 1; 
fi

suffix=
if [ "$2" = "d" ]; then
	suffix="D";
fi

start()
{
	echo "DataBackup$suffix ... start";
	cd ../cmd_tools/;
	python ./mysql_backup.py
	cd -;
}

stop()
{
	echo "DataBackup$suffix ... stop";
	logtodbPid=`ps aux |grep "mysql_backup.py"|grep -v grep | awk '{print$2}'` 
	if [ ! -n "$logtodbPid" ]; then
		echo "DataBackup: no process found"
	else
		kill -SIGTERM $logtodbPid
	fi
}

restart()
{
	stop;
	start;
}

stat()
{
	num=`ps aux | grep -w 'mysql_backup.py' | grep -w \`whoami\` | grep -v grep | wc -l`;
	if [ $num -eq 0 ]; then
		echo "[Stop] DataBackup ... attention!"
	else
		echo "[Running] DataBackup ... OK!"
	fi
}

case $1 in
	start)
	start;
	;;
	stop)
	stop;
	;;
	restart)
	restart;
	;;
	stat)
	stat;
	;;
esac

