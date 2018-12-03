#!/bin/bash

. comm.sh

app=$0
Port=$3
if [ $# -lt 3 ]; then 
    echo "usage: $0 <start|stop|restart> <r|d> <Count>";
	exit 1; 
fi

suffix=
if [ "$2" = "d" ]; then
	suffix="D";
fi

#检查是不是整数
var=$(echo $3 | bc 2>/dev/null)
if [ "$var" != "$3" ];then
    echo "the third Para is error"
    exit 1
fi


reload_tbus()
{
	echo "tbus ... start";
	cd ../services/tbusd;
	./tbusmgr --conf-file=../../config/tbus_services.xml --write;
	cd -;
}

start()
{
	
	echo "FightSvr$suffix$Port ... start";
	
	cd ../services/FightSvr;
	./FightSvr$suffix --work-dir=../../ --daemon --config-xml=FightSvr$Port.xml
	cd -;
	
	echo "FightConn$suffix$Port ... start";
	cd ../services/NetConn;
	./NetConn$suffix --work-dir=../../ --daemon --config-xml=FightConn$Port.xml
	cd -;
}

stop()
{
	echo "FightSvr$suffix$Port ... stop";
	echo `ps -ef |grep "FightSvr$Port.xml"  |awk '{print $2}'|xargs kill -9`
	echo "FightConn$suffix$Port ... stop";
	#killall -SIGTERM NetConn$suffix
	echo `ps -ef |grep "FightConn$Port.xml"  |awk '{print $2}'|xargs kill -9`
}

restart()
{
	stop;
	start;
}

stat()
{
	#monitor FightSvr$suffix;
	echo `ps -ef |grep "FightSvr$Port.xml"`
	
}


ex()
{
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
}
if [ "$1" == "start" ];then
	reload_tbus
fi
for((i=1; i<=$3; i++))
do
	Port=$((7300+$i+1))
	ex $1
done
#echo "skskslsk  $1"

