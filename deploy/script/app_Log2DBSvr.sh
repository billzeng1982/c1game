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
	echo "LogToDBSvr$suffix ... start";
	cd ../services/LogToDBSvr;
	python ./LogToDBSvr.py
	cd -;
}

stop()
{
	echo "LogToDBSvr$suffix ... stop";
	logtodbPid=`ps aux |grep "LogToDBSvr.py"|grep -v grep | awk '{print$2}'` 
	if [ ! -n "$logtodbPid" ]; then
		echo "LogToDBSvr: no process found"
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
	num=`ps aux | grep -w 'LogToDBSvr.py' | grep -w \`whoami\` | grep -v grep | wc -l`;
	if [ $num -eq 0 ]; then
		echo "[Stop] LogToDBSvr ... attention!"
	else
		echo "[Running] LogToDBSvr ... OK!"
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

