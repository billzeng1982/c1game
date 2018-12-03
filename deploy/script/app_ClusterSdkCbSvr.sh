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
	echo "ClusterSdkCbSvr$suffix ... start";
	cd ../services/ClusterSdkCbSvr;
	python ./ClusterSdkCbSvr.py
	cd -;
}

stop()
{
	echo "ClusterSdkCbSvr$suffix ... stop";
	ClusterSdkCbSvrPid=`ps aux |grep "ClusterSdkCbSvr.py"|grep -v grep | awk '{print$2}'` 
	if [ ! -n "$ClusterSdkCbSvrPid" ]; then
		echo "ClusterSdkCbSvr: no process found"
	else
		kill -SIGTERM $ClusterSdkCbSvrPid
	fi
}

restart()
{
	stop;
	start;
}

stat()
{
	num=`ps aux | grep -w 'ClusterSdkCbSvr.py' | grep -w \`whoami\` | grep -v grep | wc -l`;
	if [ $num -eq 0 ]; then
		echo "[Stop] ClusterSdkCbSvr ... attention!"
	else
		echo "[Running] ClusterSdkCbSvr ... OK!"
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

