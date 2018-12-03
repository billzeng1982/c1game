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
	echo "LogQuerySvr$suffix ... start";
	cd ../services/LogQuerySvr;
	./LogQuerySvr$suffix --work-dir=../../ --daemon
	cd -;
}

stop()
{
	echo "LogQuerySvr$suffix ... stop";
	killall -SIGTERM LogQuerySvr$suffix
}

restart()
{
	stop;
	start;
}

stat()
{
	monitor LogQuerySvr$suffix;
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

