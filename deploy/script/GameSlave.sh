#!/bin/bash

##	说明:
##		正常启动时建议先启动数据相关服务器,再启动其他逻辑服务器
##		正常关闭时,必须要先关闭逻辑服务器,等逻辑服务器把内存数据发给数据库服务器,然后再开始关闭数据库服务器,不然会丢失内存数据
##		与数据库有关的放在AppDBList上

applist=`ls app_*.sh`

#等待多少秒后停止DBSvr
WaitDBTime=1

#Monitor层级
AppMonitor=(app_ProcessMonitor.sh)
#Fight层级
AppFightList=(app_FightConn.sh app_FightSvr.sh app_MatchSvr.sh)
#Cluster层级
AppClusterList=(app_ClusterGate.sh app_DirSvr.sh app_ClusterSdkCbSvr.sh app_SerialNumSvr.sh)
#WorldLogic层级
AppWorldLogicList=(app_ZoneConn.sh app_SdkDMSvr.sh app_ZoneSvr.sh app_MailSvr.sh app_RankSvr.sh app_XiYouSDKSvr.sh app_CloneBattleSvr.sh app_IdipAgentSvr.sh app_ZoneHttpConn.sh)
#WorldData层级
AppWorldDataList=(app_FriendSvr.sh app_GuildSvr.sh app_Log2DBSvr.sh app_AccountSvr.sh app_MailDBSvr.sh app_MessageSvr.sh app_RoleSvr.sh app_MiscSvr.sh app_AsyncPvpSvr.sh app_LogQuerySvr.sh)

StartAll=(${AppWorldLogicList[*]} ${AppWorldDataList[*]})
SvrNum=${#StartAll[*]}
ParaNum=$#
flag=$2

if [ $ParaNum -lt 2 ]; then
	echo "usage1: $0 <start|stop|restart|stat> <r|d>";
	echo "usage2: $0 -fgt <start|stop|restart|stat> <r|d>"
	echo "usage3: $0 -cls <start|stop|restart|stat> <r|d>"
	echo "usage4: $0 -wl  <start|stop|restart|stat> <r|d>"
	echo "usage5: $0 -wd  <start|stop|restart|stat> <r|d>"
	exit 1;
fi

suffix="r"
if [ "$2" = "d" -o "$3" = "d" ]; then
	suffix="d";
fi

SvrType="w"

run()
{
	for key in $applist;
	do
		if [ -x $key ]; then
			bash ./$key $1 $suffix;
		fi
        
        if [ $1 != "stat" ]
        then
            sleep 1
        fi
	done
}

start()
{
    bash ./tbus.sh start $suffix;
	bash ./tlog.sh start $suffix;
    
    echo ""
    echo "start world data ..."
	applist=${AppWorldDataList[*]};
	run start;
    
    
    echo ""
    echo "start world logic ...";
	applist=${AppWorldLogicList[*]};
	run start;
	cd -
	bash ./app_ProcessMonitor.sh start $SvrType
}

stop()
{
	bash ./app_ProcessMonitor.sh stop $SvrType
    echo ""
	echo "stop world logic ...";
	applist=${AppWorldLogicList[*]};
	run stop;
	sleep $WaitDBTime;
	

	
    echo ""
	echo "stop world data ..."
	applist=${AppWorldDataList[*]};
	run stop;
	sleep 1;
	
    echo ""
	bash ./tbus.sh stop $suffix;
	bash ./tlog.sh stop $suffix;
    bash ./del_shm.sh
}

restart()
{
	stop;
	start;
}

stat()
{
	echo "stat...";
   	bash ./tbus.sh stat $suffix;
	bash ./tlog.sh stat $suffix;
	applist=${StartAll[*]};
	run stat;
}

fight()
{
	if [ $ParaNum -lt 3 ]; then
		echo "usage: $0 -fgt <start|stop|restart|stat> <r|d>"
	exit 1;
	fi
	applist=${AppFightList[*]};
	case $flag in
		start)
		run start;
		;;
		stop)
		run stop;
		;;
		stat)
		run stat;
		;;
		restart)
		run restart;
		;;
	esac
}

cluster()
{
	if [ $ParaNum -lt 3 ]; then
		echo "usage: $0 -cls <start|stop|restart|stat> <r|d>"
	exit 1;
	fi
	applist=${AppClusterList[*]};
	case $flag in
		start)
		run start;
		;;
		stop)
		run stop;
		;;
		stat)
		run stat;
		;;
		restart)
		run restart;
		;;
	esac
}

worldlogic()
{
	if [ $ParaNum -lt 3 ]; then
		echo "usage: $0 -wl <start|stop|restart|stat> <r|d>"
	exit 1;
	fi
	applist=${AppWorldLogicList[*]};
	case $flag in
		start)
		run start;
		;;
		stop)
		run stop;
		;;
		stat)
		run stat;
		;;
		restart)
		run restart;
		;;
	esac
}

worlddata()
{
	if [ $ParaNum -lt 3 ]; then
		echo "usage: $0 -wd <start|stop|restart|stat> <r|d>"
	exit 1;
	fi
	applist=${AppWorldDataList[*]};
	case $flag in
		start)
		run start;
		;;
		stop)
		run stop;
		;;
		stat)
		run stat;
		;;
		restart)
		run restart;
		;;
	esac
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
