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
	echo "GuildExpeditionSvr$suffix ... start";
	cd ../services/GuildExpeditionSvr;
	./GuildExpeditionSvr$suffix --work-dir=../../ --daemon --sample
	cd -;
}

stop()
{
	echo "GuildExpeditionSvr$suffix ... stop";
	killall -SIGTERM GuildExpeditionSvr$suffix
}

restart()
{
	stop;
	start;
}

stat()
{
	monitor GuildExpeditionSvr$suffix;
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

