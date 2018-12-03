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
	echo "FightConn$Port$suffix ... start";
	cd ../services/NetConn;
	./NetConn$suffix --work-dir=../../ --daemon --sample --config-xml="FightConn$Port.xml"
	cd -;
}

stop()
{
	echo "FightConn$Port$suffix ... stop";
	killall -SIGTERM NetConn$suffix
}

restart()
{
	stop;
	start;
}

stat()
{
	monitor NetConn$suffix;
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

