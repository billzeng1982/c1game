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
	echo "tbus ... start";
	cd ../services/tbusd;
	./tbusmgr --conf-file=../../config/tbus_services.xml --write;
    ./trelaymgr --conf-file=../../config/trelaymgr.xml --write;
	./tbusd --pid-file=../../pid/tbusd.pid  --conf-file=../../config/tbusd.xml --tlogconf=../../config/tbusd_log.xml --epoll-wait=5 -D start 
	cd -;
}

stop()
{
	echo "tbus ... stop";
	cd ../services/tbusd;
	./tbusd --pid-file=../../pid/tbusd.pid stop
	./tbusmgr --conf-file=../../config/tbus_services.xml --delete-all;
	./trelaymgr --conf-file=../../config/trelaymgr.xml --delete-all;
	cd -;
}

restart()
{
	stop;
	start;
}

stat()
{
	monitor tbusd
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

