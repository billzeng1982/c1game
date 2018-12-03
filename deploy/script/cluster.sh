#!/bin/bash

##	cluster层级启动、停止脚本

stime=1

AppList=(app_ClusterGate.sh app_SdkDMSvr.sh app_DirSvr.sh app_ClusterSdkCbSvr.sh)

if [ $# -lt 2 ]; then
	echo "usage: $0 <start|stop|restart|stat> <r|d>";
	exit 1;
fi

applist=${AppList[*]}

suffix="r"
if [ "$2" = "d" ]; then
	suffix="d";
fi

run()
{
	for key in $applist;
	do
		if [ -x $key ]; then
			bash ./$key $1 $suffix;
			sleep $stime;
		fi
	done
}

start()
{
	echo "start...";
	run start;
}

stop()
{
	echo "stop...";
	run stop;
}

restart()
{
	stop;
	start;
}

stat()
{
	echo "stat...";
	run stat;
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
