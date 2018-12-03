#!/bin/bash

## supervisor的启停脚本

if [ $# -lt 1 ]; then
	echo "usage1: $0 <start|stop|restart|status>";
	exit 1;
fi

case $1 in
	start)
	echo "supervisor start ..."
	/usr/bin/service supervisor start
	;;
	stop)
	echo "supervisor stop ..."
	/usr/bin/service supervisor stop
	;;
	restart)
	echo "supervisor restart ..."
	/usr/bin/service supervisor restart
	;;
	status)
	/usr/bin/service supervisor status
	;;
    esac
