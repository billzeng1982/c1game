#!/bin/sh

. ./comm.sh

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
	echo "tlogd ... start";
	cd ../services/tlogd;
	./tlogd  --log-file=./tlogd --pid-file=../../pid/tlogd.pid --daemon  start;
	cd -;
}

stop()
{
	echo "tlogd ... stop";
	cd ../services/tlogd;
	./tlogd --pid-file=../../pid/tlogd.pid stop;
	cd -;
}

restart()
{
	stop;
	start;
}

stat()
{
	monitor tlogd
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
