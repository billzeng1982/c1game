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
	echo "ZoneSvr$suffix ... start";
	cd ../services/ZoneSvr;
	./ZoneSvr$suffix --work-dir=../../ --daemon --sample
	cd -;
}

stop()
{
	echo "ZoneSvr$suffix ... stop";
	killall -SIGTERM ZoneSvr$suffix
}

restart()
{
	stop;
	start;
}

stat()
{
	monitor ZoneSvr$suffix;
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

