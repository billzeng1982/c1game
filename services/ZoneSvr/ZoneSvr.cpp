#include <fstream>
#include "ZoneSvr.h"
#include "workdir.h"
#include "oi_misc.h"
#include "strutil.h"
#include "LogMacros.h"
#include "ObjectUpdatorMgr.h"
#include "GameTime.h"
#include "framework/GameObjectPool.h"
#include "framework/ZoneSvrMsgLayer.h"
#include "gamedata/GameDataMgr.h"
#include "module/player/PlayerMgr.h"
#include "module/FightPVP.h"
#include "module/Task.h"
#include "module/Lottery.h"
#include "module/Tutorial.h"
#include "module/FightStatistics.h"
#include "module/AP.h"
#include "module/FightPVE.h"
#include "module/Majesty.h"
#include "module/ZoneLog.h"
#include "module/Guild.h"
#include "module/Sign.h"
#include "module/Equip.h"
#include "module/GeneralCard.h"
#include "module/RankMgr.h"
#include "module/Mail.h"
#include "module/ActivityMgr.h"
#include "GameTime.h"
#include "module/Gm/Gm.h"
#include "module/Affiche.h"
#include "module/Marquee.h"
#include "module/VIP.h"
#include "module/Mall.h"
#include "module/League.h"
#include "module/LevelRank.h"
#include "module/Pay.h"
#include "module/AsyncPvp.h"
#include "module/Shops.h"
#include "module/SkillPoint.h"
#include "module/DailyChallenge.h"
#include "module/SvrTime.h"
#include "module/MasterSkill.h"
#include "module/TimeCycle.h"
#include "module/Friend.h"
#include "module/PeakArena.h"
#include "module/CloneBattle.h"
#include "module/Message.h"
#include "module/GuildGeneralHang.h"
#include "module/TaskAct.h"
#include "module/Tactics.h"
#include "module/Mine.h"
#include "module/GeneralReborn.h"

using namespace std;
extern unsigned char g_szMetalib_ZoneSvrCfg[];

#define CLOSE_TIME_FILE_NAME "closetime.rec"

ZoneSvr::ZoneSvr() : m_ullSvrOfflineLastTime(0), m_dwIntervalPass(0), m_stSvrInfoSendBuf(sizeof(DT_SERVER_INFO)*2 +1)
{
    m_ullLastUptTime = CGameTime::Instance().GetCurrSecond();
}

bool ZoneSvr::AppInit()
{
    _ReadInfoFromFile();

    if (!ZoneSvrMsgLayer::Instance().Init())
    {
        LOGERR("ZoneSvrMsgLayer init error.");
        return false;
    }

    if (!CGameDataMgr::Instance().Init())
    {
        LOGERR("CGameDataMgr init error.");
        return false;
    }

    if (!m_oUdpRelayClt.Init())
    {
        LOGERR("UdpRelayClt init error");
        return false;
    }

    OnReleaseTimerCb_t fReleaseTimer = ZoneSvr::ReleaseTimer;
    if (!m_oLogicTimerMgr.Init(m_iLogicTimerMaxNum, fReleaseTimer))
    {
        LOGERR("LogicTimerMgr init error.");
        return false;
    }
    if (!GmMgr::Instance().Init())
    {//Gm操作的所存的文件 先加载
        LOGERR("GmMgr init error");
        return false;
    }

    if (!SvrTime::Instance().Init())
    {
        LOGERR("SvrTime Init failed");
        return false;
    }

    if (!PlayerMgr::Instance().Init(m_stConfig.m_iMaxOnlinePlayer))
    {
        LOGERR("PlayerMgr init error.");
        return false;
    }

    if (!Fight6V6::Instance().Init())
    {
        LOGERR("FightPVP init error.");
        return false;
    }

    if (!Task::Instance().Init())
    {
        LOGERR("Task init error.");
        return false;
    }

    if (!Lottery::Instance().Init())
    {
        LOGERR("Lottery init error.");
        return false;
    }

    if (!Tutorial::Instance().Init())
    {
        LOGERR("Tutorial init error.");
        return false;
    }

    if (!Majesty::Instance().Init())
    {
        LOGERR("AP init error.");
        return false;
    }

    if (!AP::Instance().Init())
    {
        LOGERR("AP init error.");
        return false;
    }

    if (!ZoneLog::Instance().Init(m_stConfig.m_szLogCfgPath, m_stConfig.m_szCltLogTdrPath, m_stConfig.m_szSvrLogTdrPath))
    {
        LOGERR("ZoneLog init error.");
        return false;
    }

    if (!Guild::Instance().Init())
    {
        LOGERR("Guild init error");
        return false;
    }

    if (!Sign::Instance().Init())
    {
        LOGERR("Sign init error");
        return false;
    }

    if (!Equip::Instance().Init())
    {
        LOGERR("Equip init error");
        return false;
    }

    if (!GeneralCard::Instance().Init())
    {
        LOGERR("GeneralCard init error");
        return false;
    }

    if (!GeneralReborn::Instance().Init())
    {
        LOGERR("GeneralReborn init error");
        return false;
    }

    if (!RankMgr::Instance().Init())
    {
        LOGERR("RankMgr init error");
        return false;
    }

    if (!Marquee::Instance().Init())
    {
        LOGERR("Marquee init error!");
        return false;
    }

    if (!Affiche::Instance().Init())
    {
        LOGERR("Affiche init error");
        return false;
    }

    if (!ActivityMgr::Instance().Init())
    {
		LOGERR("Activity init error");
    }

    if (!VIP::Instance().Init())
    {
        LOGERR("VIP init error");
        return false;
    }
    if (! Mall::Instance().Init())
    {
        LOGERR("Mall init error");
        return false;
    }
    if (! League::Instance().Init())
    {
        LOGERR("League init error");
        return false;
    }
    if (! LevelRank::Instance().Init(&m_stConfig))
    {
        LOGERR("LevelRank init error");
        return false;
    }

    if (! Pay::Instance().Init())
    {
        LOGERR("Pay init error");
        return false;
    }
    if (! AsyncPvp::Instance().Init())
    {
        LOGERR("AsyncPvp init error");
        return false;
    }
	if (! Shops::Instance().Init())
	{
		LOGERR("Shops init error");
		return false;
	}
    if (! SkillPoint::Instance().Init())
    {
        LOGERR("SkillPoint init error");
        return false;
    }
	if (! DailyChallenge::Instance().Init())
    {
       LOGERR("DailyChallenge init error");
       return false;
    }
    if (!MasterSkill::Instance().Init())
    {
        LOGERR("MasterSkill init error");
        return false;
    }
    if (!TimeCycle::Instance().Init())
    {
        LOGERR("TimeCycle init error");
        return false;
    }
	if (!Friend::Instance().Init())
	{
		LOGERR("Friend init error");
		return false;
	}
    if (!PeakArena::Instance().Init())
    {
        LOGERR("PeakArena init error");
        return false;
    }
    if (!CloneBattle::Instance().Init())
    {
        LOGERR("CloneBattle init error");
        return false;
    }

    if (!Message::Instance().Init())
    {
        LOGERR("Message init error");
        return false;
    }

	if (!GuildGeneralHang::Instance().Init())
	{
		LOGERR("GuildGeneralHang init error");
		return false;
	}

    if (!TaskAct::Instance().Init())
    {
        LOGERR("TaskAct init error");
        return false;
    }

    if (!Tactics::Instance().Init())
    {
        LOGERR("Tactics init error");
        return false;
    }

    if (!Mine::Instance().Init())
    {
        LOGERR("Tactics init error");
        return false;
    }

    this->_InitSvrInfo();

    return true;
}

void ZoneSvr::AppFini()
{
    _NotifyClusterGate(0);
    _NotifyDirSvr(0);
    _WriteInfoToFile();

    GmMgr::Instance().Fini();
    PlayerMgr::Instance().LogoutAllPlayers();
    CAppFrame::AppFini();
    LevelRank::Instance().Fini();
    Pay::Instance().Fini();
}

void ZoneSvr::AppUpdate()
{
    bool bIdle = true;

    // 数据包的处理
    if (ZoneSvrMsgLayer::Instance().DealPkg() > 0)
    {
        bIdle = false;
    }

    // 定时器，用于服务器自身数据的更新
    //m_oLogicTimerMgr.Update();

    Task::Instance().UpdateServer();
    FightPVE::Instance().UpdateServer();
    Fight6V6::Instance().UpdateServer();
    Guild::Instance().UpdateServer();
    ActivityMgr::Instance().UpdateServer();
    Marquee::Instance().UpdateServer();
    VIP::Instance().UpdateServer();
    Mall::Instance().UpdateServer();
    RankMgr::Instance().Update();
    League::Instance().UpdateServer();
	LevelRank::Instance().UpdateServer();
    Pay::Instance().UpdateServer();
    AsyncPvp::Instance().UpdateServer();
	Shops::Instance().UpdateServer();
	DailyChallenge::Instance().UpdateServer();
    Friend::Instance().UpdateServer();
    PeakArena::Instance().UpdateServer();
    TaskAct::Instance().Update();
    Mine::Instance().UpdateServer();
    // 玩家数据的更新
    PlayerMgr::Instance().Update(bIdle);

    // notify clustergate state
    m_dwIntervalPass = (uint32_t)(CGameTime::Instance().GetCurrSecond() - m_ullLastUptTime);
    if (m_dwIntervalPass >= m_stConfig.m_dwReportInterval)
    {
        m_ullLastUptTime = CGameTime::Instance().GetCurrSecond();
        _NotifyClusterGate(1);
        _NotifyDirSvr(1);
    }

    if (bIdle)
    {
        MsSleep(1);
    }
}

void ZoneSvr::ReleaseTimer(GameTimer* pTimer)
{
    if (!pTimer)
    {
        return;
    }

    // 释放Timer
    RELEASE_GAMEOBJECT(pTimer);

    return;
}

void ZoneSvr::_SetAppCfg()
{
    m_stAppCfg.m_pszBuff = (char*) &m_stConfig;
    m_stAppCfg.m_dwLen = sizeof(m_stConfig);
    m_stAppCfg.m_pstMetaLib = (LPTDRMETALIB) g_szMetalib_ZoneSvrCfg;
    StrCpy(m_stAppCfg.m_szMetaName, "ZoneSvrCfg", sizeof(m_stAppCfg.m_szMetaName));
    snprintf(m_stAppCfg.m_szCfgFile, sizeof(m_stAppCfg.m_szCfgFile), "%s/config/ZoneSvr.xml", CWorkDir::string());
}

bool ZoneSvr::_ReadCfg()
{
    if (!CAppFrame::_ReadCfg())
    {
        return false;
    }

    inet_aton(m_stConfig.m_szProcIDStr, (struct in_addr*) &m_stConfig.m_iProcID);
    inet_aton(m_stConfig.m_szConnIDStr, (struct in_addr*) &m_stConfig.m_iConnID);

    inet_aton(m_stConfig.m_szAccountSvrIDStr, (struct in_addr*) &m_stConfig.m_iAccountSvrID);
    inet_aton(m_stConfig.m_szRoleSvrIDStr, (struct in_addr*) &m_stConfig.m_iRoleSvrID);
    inet_aton(m_stConfig.m_szClusterGateIDStr, (struct in_addr*) &m_stConfig.m_iClusterGateID);
    inet_aton(m_stConfig.m_szRankSvrIDStr, (struct in_addr*) &m_stConfig.m_iRankSvrID);
    inet_aton(m_stConfig.m_szMessageSvrIDStr, (struct in_addr*) &m_stConfig.m_iMessageSvrID);
    inet_aton(m_stConfig.m_szGuildSvrIDStr, (struct in_addr*) &m_stConfig.m_iGuildSvrID);
    inet_aton(m_stConfig.m_szMailSvrIDStr, (struct in_addr*) &m_stConfig.m_iMailSvrID);
    inet_aton(m_stConfig.m_szFriendSvrIDStr, (struct in_addr*) &m_stConfig.m_iFriendSvrID);
    inet_aton(m_stConfig.m_szReplaySvrIDStr, (struct in_addr*) &m_stConfig.m_iReplaySvrID);
    inet_aton(m_stConfig.m_szXiYouSDKSvrIDStr, (struct in_addr*) &m_stConfig.m_iXiYouSDKSvrID);
    inet_aton(m_stConfig.m_szSdkDMSvrIDStr, (struct in_addr*) &m_stConfig.m_iSdkDMSvrID);
    inet_aton(m_stConfig.m_szSerialNumSvrIDStr, (struct in_addr*)&m_stConfig.m_iSerialNumSvrID);
    inet_aton(m_stConfig.m_szSvrIpStr, (struct in_addr*)&m_stConfig.m_iSvrIp);
	inet_aton(m_stConfig.m_szMiscIDStr, (struct in_addr*)&m_stConfig.m_iMiscID);
    inet_aton(m_stConfig.m_szAsyncPvpSvrIDStr, (struct in_addr*)&m_stConfig.m_iAsyncPvpSvrID);
    inet_aton(m_stConfig.m_szCloneBattleSvrIDStr, (struct in_addr*)&m_stConfig.m_iCloneBattleSvrID);
    inet_aton(m_stConfig.m_szIdipAgentSvrIDStr, (struct in_addr*)&m_stConfig.m_iIdipAgentSvrID);
    inet_aton(m_stConfig.m_szZoneHttpConnIDStr, (struct in_addr*)&m_stConfig.m_iZoneHttpConnID);
    return true;
}

void ZoneSvr::_ReadInfoFromFile()
{
    // 读取上次关闭时间
    ifstream oFile;
    oFile.open (CLOSE_TIME_FILE_NAME, ios::in | ios::binary);
    if (oFile.is_open())
    {
        oFile.read ((char*)&m_ullSvrOfflineLastTime, sizeof(m_ullSvrOfflineLastTime));
        oFile.close();
    }
    else
    {
        // 更新时间戳，暂时只存关闭时间，不用结构体
        m_ullSvrOfflineLastTime = CGameTime::Instance().GetCurrTimeMs();
    }
    return;
}

void ZoneSvr::_NotifyClusterGate(uint8_t bOnline)
{
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_CLUSTER_SERV_STAT_NTF;
	m_stSsPkg.m_stHead.m_ullReservId = 0;
    DT_SERVER_INFO& rstSvrInfo = m_stSsPkg.m_stBody.m_stClusterServStatNtf.m_stServInfo;

    rstSvrInfo.m_bOnline = bOnline;
    rstSvrInfo.m_iProcId = m_stConfig.m_iProcID;
    rstSvrInfo.m_dwSvrId = m_stConfig.m_wSvrId;
    rstSvrInfo.m_iIp = m_stConfig.m_iSvrIp;
    rstSvrInfo.m_wPort = m_stConfig.m_wSvrPort;
    rstSvrInfo.m_dwLoad = PlayerMgr::Instance().GetUsedNum();
    memcpy(rstSvrInfo.m_szSvrName, m_stConfig.m_szSvrName, PKGMETA::MAX_NAME_LENGTH);

    ZoneSvrMsgLayer::Instance().SendToClusterGate(m_stSsPkg);
}

void ZoneSvr::_InitSvrInfo()
{
    m_stServerInfo.m_dwSvrId = m_stConfig.m_wSvrId;
    m_stServerInfo.m_iIp = m_stConfig.m_iSvrIp;
    m_stServerInfo.m_wPort = m_stConfig.m_wSvrPort;
    memcpy(m_stServerInfo.m_szSvrName, m_stConfig.m_szSvrName, PKGMETA::MAX_NAME_LENGTH);
}

void ZoneSvr::_NotifyDirSvr(uint8_t bOnline)
{
    /*m_stServerInfo.m_bOnline = bOnline;
    m_stServerInfo.m_dwLoad = PlayerMgr::Instance().GetUsedNum();
    m_stSvrInfoSendBuf.Reset();
    int iRet = m_stServerInfo.pack(m_stSvrInfoSendBuf.m_szTdrBuf, m_stSvrInfoSendBuf.m_uSize, &m_stSvrInfoSendBuf.m_uPackLen, 0);
    if (iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR("_NotifyDirSvr failed, pack SvrInfo error!Ret-%d", iRet);
        return;
    }

    iRet = m_oUdpRelayClt.Send(m_stSvrInfoSendBuf.m_szTdrBuf, m_stSvrInfoSendBuf.m_uPackLen, m_stConfig.m_szDirSvrIpStr, m_stConfig.m_wDirSvrPort);
    if (iRet != (int)m_stSvrInfoSendBuf.m_uPackLen)
    {
        LOGERR("_NotifyDirSvr failed, Udp Send SvrInfo error!PkgLen-%lu, SendLen-%d", m_stSvrInfoSendBuf.m_uPackLen, iRet);
        return ;
    }*/
}

void ZoneSvr::_WriteInfoToFile()
{
    // 更新时间戳
    m_ullSvrOfflineLastTime = CGameTime::Instance().GetCurrTimeMs();

    // 写入文件
    ofstream oFile;
    oFile.open (CLOSE_TIME_FILE_NAME, ios::out | ios::binary);
    if (oFile.is_open())
    {
        oFile.write((char*)&m_ullSvrOfflineLastTime, sizeof(m_ullSvrOfflineLastTime));
        oFile.close();
    }
    return;
}


