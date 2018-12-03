#!/bin/bash

#使用前先把脚本软链接到  /usr/local/bin
##	ln -s `pwd`/raye.sh /usr/local/bin/raye
##	chmod a+x `pwd`/raye.sh


#外网建议只留切换目录命令

#使用如下:
#	raye c  #编译整个服务器,make日志会保存在编译目录的er文件
#	raye s  #启动服务器
#   . raye cd RoleSvr  ## 切换目录需加点操作
#	其他命令参考代码或usage
##注意:
#	切换目录命令会改变当前shell的环境变量


SERVER_DIR=`dirname $(readlink -f ${BASH_SOURCE[0]})`

SERVICE_DIR=$SERVER_DIR/services
SELF=raye
SVR_COMPILE_TYPE=d
COMPILE_DIR=$SERVER_DIR
SVR_ALL_COMPILE=(
    AccountSvr
    AsyncPvpSvr
    ClusterGate
    FightSvr
    FriendSvr
    GuildSvr
    Log2DBSvr
    MailDBSvr
    MailSvr
    MatchSvr
    MiscSvr
    MessageSvr
    NetConn
    ProxySvr
    RankSvr
    ReplaySvr
    RoleSvr
	SerialNumSvr
    XiYouSDKSvr
    SerialNumSvr
    CloneBattleSvr
    ZoneSvr
    MineSvr
    MineDBSvr
    GuildExpeditionSvr
    ClusterDBSvr
	Com
	Log
    GuildExpeditionSvr
    ZoneHttpConn
)
PARA=($@)
PARA_LEN=$#

usage()
{
	echo "raye::explain"
    echo "$SELF c#compile|cl#clean|s#start|rs#restart|stt#stat|sp#stop|up#svn_update [SvrName]"
	echo ". $SELF cd [SvrName] #cd path"
	echo "SvrName=${SVR_ALL_COMPILE[*]}"
}
check_service()
{
    local tmp
    for tmp in ${SVR_ALL_SERVICE[*]};do
        if [ "${PARA[1]}" = "$tmp" ];then
            return 0
        else
            return 1
        fi
    done
}
check_svr()
{
    local tmp
    for tmp in ${SVR_ALL_COMPILE[*]};do
        if [ "${PARA[1]}" = "$tmp" ];then
            return 0
        else
            continue
        fi
    done
	return 1
}

make_fun()
{
    cd $COMPILE_DIR
	echo `pwd`
    echo "start compile ${PARA[1]}"
    make -j4 &>`pwd`/er
	local ret=`grep "err" er | wc -l`
    if [ "$ret" = "0" ];then
        echo "compile ${PARA[1]} ok"
    else
        echo "compile ${PARA[1]} error XXXXXXXXXXXXXX"
		grep -A5 -B3 "error" er
    fi
}

compile()
{
    if [ $PARA_LEN == 1 ];then
		COMPILE_DIR=$SERVER_DIR
        make_fun
    else
        check_svr
        if [ "$?" = "0" ];then
			if [ "${PARA[1]}" = "Com" ];then
				COMPILE_DIR=$SERVER_DIR/common
			else
				COMPILE_DIR=$SERVICE_DIR/${PARA[1]}
			fi
            make_fun
        else
            echo "cmd error"
            usage
        fi
    fi
}

clean()
{
	local target_dir
	if [ $PARA_LEN == 1 ];then
        target_dir=$SERVER_DIR
    else
		check_svr
		if [ "$?" = "0" ];then
			if [ "${PARA[1]}" = "Com" ];then
				target_dir=$SERVER_DIR/common
			else
				target_dir=$SERVICE_DIR/${PARA[1]}
			fi
        else
            echo "cmd error"
            usage
			return 1
        fi
	fi
    cd $target_dir
	echo `pwd`
	echo "start clean ${PARA[1]} "
    make clean &>/dev/null
	if [ "$?" = "0" ];then
        echo "clean ${PARA[1]} ok"
    else
        echo "clean ${PARA[1]} error"
    fi
}
svr_ex()
{
	cd $SERVER_DIR/deploy/script
	echo `pwd`
	local shell
	if [ $PARA_LEN == 1 ];then
		shell=gamesvr.sh
	else
		check_svr
		if [ "$?" = "0" ];then
			shell=app_${PARA[1]}.sh
        else
			echo "cmd error"
			usage
			return 1
		fi
	fi
	./$shell $1   $SVR_COMPILE_TYPE
	./$shell stat   $SVR_COMPILE_TYPE
}

svr_start()
{
	svr_ex start
}

svr_stat()
{
    cd $SERVER_DIR/deploy/script
	echo `pwd`
    ./gamesvr.sh stat $SVR_COMPILE_TYPE
}
svr_stop()
{
    svr_ex stop
}
svr_restart()
{
    svr_ex restart
}
svn_update()
{
	cd $SERVER_DIR
	echo `pwd`
	svn up
	if [ "$?" = "0" ];then
        echo "svn_update ok"
    else
        echo "svn_update error"
    fi
}
cd_path()
{
	local target_dir=$SERVER_DIR

	if [ $PARA_LEN == 1 ];then
		target_dir=$SERVER_DIR
	else
		check_svr
		if [ "$?" = "0" ];then
			if [ "${PARA[1]}" = "Com" ];then
				target_dir=$SERVER_DIR/common
			elif [ "${PARA[1]}" = "Log" ];then
				target_dir=$SERVER_DIR/deploy/log
			else
				target_dir=$SERVICE_DIR/${PARA[1]}
			fi
		else
			echo "cmd error"
			usage
			return 1
		fi
	fi
	cd $target_dir

}

case ${PARA[0]} in
    c)
        # complie the all svr
        compile
        ;;
    cl)
        # clean the all svr obj
        clean
        ;;
    rc)
        clean
        compile
        ;;
    s)
        # start the project
        svr_start
        ;;
	stt)
		# look up the svr stat
		svr_stat
		;;
	sp)
		# stop the svr
		svr_stop
		;;
    rs)
        # restart the project
        svr_restart
        ;;
    up)
        # svn update the project
        svn_update
        ;;
	cdd)
		# cd the path
		cd_path
		;;
    *)
        usage
        ;;
esac


