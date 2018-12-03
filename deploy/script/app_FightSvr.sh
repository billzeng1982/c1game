#!/bin/bash

. comm.sh

app=$0
Port=$3
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
	echo "FightSvr$suffix ... start";
	cd ../services/FightSvr;
	./FightSvr$suffix --work-dir=../../ --daemon --sample --config-xml=FightSvr$Port.xml
	cd -;
}

stop()
{
	echo "FightSvr$suffix ... stop";
	killall -SIGTERM FightSvr$suffix
}

restart()
{
	stop;
	start;
}

stat()
{
	monitor FightSvr$suffix;
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

