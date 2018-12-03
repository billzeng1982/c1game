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
	echo "DirSvr$suffix ... start";
	cd ../services/DirSvr;
	python ./DirSvr.py
	cd -;
}

stop()
{
    echo "DirSvr$suffix ... stop";
	DirSvrPid=`ps aux |grep "DirSvr.py"|grep -v grep | awk '{print$2}'` 
	if [ ! -n "$DirSvrPid" ]; then
		echo "DirSvr: no process found"
	else
		kill -SIGTERM $DirSvrPid
	fi
}

restart()
{
	stop;
	start;
}

stat()
{
	num=`ps aux | grep -w 'DirSvr.py' | grep -w \`whoami\` | grep -v grep | wc -l`;
	if [ $num -eq 0 ]; then
		echo "[Stop] DirSvr ... attention!"
	else
		echo "[Running] DirSvr ... OK!"
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

