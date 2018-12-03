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
	echo "MineDBSvr$suffix ... start";
	cd ../services/MineDBSvr;
	./MineDBSvr$suffix --work-dir=../../ --daemon --sample
	cd -;
}

stop()
{
	echo "MineDBSvr$suffix ... stop";
	killall -SIGTERM MineDBSvr$suffix
}

restart()
{
	stop;
	start;
}

stat()
{
	monitor MineDBSvr$suffix;
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

