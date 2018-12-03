#!/bin/bash

. comm.sh

app=$0

if [ $# -lt 2 ]; then 
    echo "usage: $0 <start|stop|restart> <w(for world)|c(for cluster)>";
	exit 1; 
fi

suffix="cluster"
if [ "$2" = "w" ]; then
	suffix="world";
fi

start()
{
	echo "ProcessMonitor$suffix ... start";
	cd ../../monitor/;	
	python ./ProcessMonitor.py --debug=0 --config-file=config_$suffix.json
	cd -;
}

stop()
{
	echo "ProcessMonitor$suffix ... stop";
	pmPid=`ps aux |grep "ProcessMonitor.py"|grep -v grep | awk '{print$2}'` 
	if [ ! -n "$pmPid" ]; then
		echo "ProcessMonitorSvr: no process found"
	else
		kill -SIGTERM $pmPid
	fi
}

restart()
{
	stop;
	start;
}

stat()
{
	num=`ps aux | grep -w 'ProcessMonitor.py' | grep -w \`whoami\` | grep -v grep | wc -l`;
	if [ $num -eq 0 ]; then
		echo "[Stop] ProcessMonitor ... attention!"
	else
		echo "[Running] ProcessMonitor ... OK!"
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

