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
	echo "AsyncPvpSvr$suffix ... start";
	cd ../services/AsyncPvpSvr;
    if [ ! -f PlayerRankList ];then
        ./GenFakePlayer PlayerRankList 7999
    fi
	./AsyncPvpSvr$suffix --work-dir=../../ --daemon --sample
	cd -;
}

stop()
{
	echo "AsyncPvpSvr$suffix ... stop";
	killall -SIGTERM AsyncPvpSvr$suffix
}

restart()
{
	stop;
	start;
}

stat()
{
	monitor AsyncPvpSvr$suffix;
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

