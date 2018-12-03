#!/usr/bin/env sh

SvrList=services/SerialNumSvr/SerialNumSvr,\
services/FriendSvr/FriendSvr,\
services/FightSvr/FightSvr,\
services/RoleSvr/RoleSvr,\
services/MiscSvr/MiscSvr,\
services/MailSvr/MailSvr,\
services/ZoneSvr/ZoneSvr,\
services/CloneBattleSvr/CloneBattleSvr,\
services/ReplaySvr/ReplaySvr,\
services/GuildSvr/GuildSvr,\
services/AsyncPvpSvr/AsyncPvpSvr,\
services/MailDBSvr/MailDBSvr,\
services/AccountSvr/AccountSvr,\
services/DirSvr/DirSvr,\
services/MatchSvr/MatchSvr,\
services/SdkDMSvr/SdkDMSvr,\
services/XiYouSDKSvr/XiYouSDKSvr,\
services/RankSvr/RankSvr,\
services/MessageSvr/MessageSvr,\
services/ClusterGate/ClusterGate,\
services/NetConn/NetConn,\
services/LogQuerySvr/LogQuerySvr,\
services/IdipAgentSvr/IdipAgentSvr,\
services/ClusterSdkCbSvr/ClusterSdkCbSvr.py,\
services/MineDBSvr/MineDBSvr,\
services/MineSvr/MineSvr

SvrDList=services/SerialNumSvr/SerialNumSvrD,\
services/FriendSvr/FriendSvrD,\
services/FightSvr/FightSvrD,\
services/RoleSvr/RoleSvrD,\
services/MiscSvr/MiscSvrD,\
services/MailSvr/MailSvrD,\
services/ZoneSvr/ZoneSvrD,\
services/CloneBattleSvr/CloneBattleSvrD,\
services/ReplaySvr/ReplaySvrD,\
services/GuildSvr/GuildSvrD,\
services/AsyncPvpSvr/AsyncPvpSvrD,\
services/MailDBSvr/MailDBSvrD,\
services/AccountSvr/AccountSvrD,\
services/DirSvr/DirSvrD,\
services/MatchSvr/MatchSvrD,\
services/SdkDMSvr/SdkDMSvrD,\
services/XiYouSDKSvr/XiYouSDKSvrD,\
services/RankSvr/RankSvrD,\
services/MessageSvr/MessageSvrD,\
services/ClusterGate/ClusterGateD,\
services/NetConn/NetConnD,\
services/LogQuerySvr/LogQuerySvrD,\
services/IdipAgentSvr/IdipAgentSvrD,\
services/ClusterSdkCbSvr/ClusterSdkCbSvr.py,\
services/MineDBSvr/MineDBSvrD,\
services/MineSvr/MineSvrD




python tardeploy.py -s --dname=$SvrDList --dir=protocol/,gamedata/
