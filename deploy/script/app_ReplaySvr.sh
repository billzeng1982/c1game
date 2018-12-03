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
	echo "ReplaySvr$suffix ... start";
	cd ../services/ReplaySvr;
	./ReplaySvr$suffix --work-dir=../../ --daemon --sample
	cd -;
}

stop()
{
	echo "ReplaySvr$suffix ... stop";
	killall -SIGTERM ReplaySvr$suffix
}

restart()
{
	stop;
	start;
}

stat()
{
	monitor ReplaySvr$suffix;
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

