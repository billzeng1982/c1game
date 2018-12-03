#!/bin/bash

##线上服部署完成后删除本脚本,防止误操作

user=root
psword=c1game123

#创建数据库
mysql  -u$user -p$psword --default-character-set=utf8 < create_database.sql

#清理表
mysql  -u$user -p$psword --default-character-set=utf8 < drop_c1ame_tables.sql

#创建表
mysql  -u$user -p$psword --default-character-set=utf8 < create_c1game_tables.sql

#删除文件数据
DataFileList=(
	../services/RankSvr/RankLiTable
	../services/RankSvr/RankDailyChallengeTable
	../services/RankSvr/RankGCardCntTable
	../services/RankSvr/RankGCardLiTable
	../services/RankSvr/RankPveStarTable
	../services/RankSvr/RankGuildTable
	../services/RankSvr/RankGFightTable
	../services/RankSvr/RankPeakArena
	../services/GuildSvr/GuildFightSchedule
	../services/GuildSvr/GuildBossSchedule
	../services/ReplaySvr/ReplayList
	../services/AsyncPvpSvr/PlayerRankList
	../services/ZoneSvr/discount_info
)

for dataFile in ${DataFileList[@]}
do
    if [ -f ${dataFile} ]; then 
		rm ${dataFile}
	fi 
done

##清除好友信息
redis-cli del PlayerInfo
#redis-cli FLUSHALL
