#!/bin/bash

#applist=`ls app_1*.sh app_3*.sh`


if [ $# -lt 2 ]; then
	echo "usage: $0 <start|stop|restart|stat> <r|d>";
	exit 1;
fi

suffix="r"
if [ "$2" = "d" ]; then
	suffix="d";
fi

stime=0

run()
{
	for key in $applist;
	do
		bash ./$key $1 $suffix;
		sleep $stime;
	done
}

start()
{
	echo "start...";
	stime=1;
	bash ./tbus.sh start $suffix;
	run start;
}

stop()
{
	echo "stop...";
	stime=1;
	run stop;
	bash ./tbus.sh stop $suffix;
    bash ./del_shm.sh
}

restart()
{
	stop;
	start;
}

stat()
{
	echo "stat...";
   	bash ./tbus.sh stat $suffix;
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
