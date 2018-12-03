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
	echo "MiscSvr$suffix ... start";
	cd ../services/MiscSvr;
	./MiscSvr$suffix --work-dir=../../ --daemon --sample
	cd -;
}

stop()
{
	echo "MiscSvr$suffix ... stop";
	killall -SIGTERM MiscSvr$suffix
}

restart()
{
	stop;
	start;
}

stat()
{
	monitor MiscSvr$suffix;
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

