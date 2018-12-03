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
	echo "MailSvr$suffix ... start";
	cd ../services/MailSvr;
	./MailSvr$suffix --work-dir=../../ --daemon --sample
	cd -;
}

stop()
{
	echo "MailSvr$suffix ... stop";
	killall -SIGTERM MailSvr$suffix
}

restart()
{
	stop;
	start;
}

stat()
{
	monitor MailSvr$suffix;
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

