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
	echo "RankSvr$suffix ... start";
	cd ../services/RankSvr;
	./RankSvr$suffix --work-dir=../../ --daemon --sample
	cd -;
}

stop()
{
	echo "RankSvr$suffix ... stop";
	killall -SIGTERM RankSvr$suffix
	test=1
}

restart()
{
	stop;
	start;
}

stat()
{
	monitor RankSvr$suffix;
	test=1
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

