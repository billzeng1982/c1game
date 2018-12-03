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
	echo "GuildSvr$suffix ... start";
	cd ../services/GuildSvr;
	./GuildSvr$suffix --work-dir=../../ --daemon --sample
	cd -;
}

stop()
{
	echo "GuildSvr$suffix ... stop";
	killall -SIGTERM GuildSvr$suffix
}

restart()
{
	stop;
	start;
}

stat()
{
	monitor GuildSvr$suffix;
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

