#include "define.h"
#include "FightSvr.h"
#include "workdir.h"
#include "conf_xml.h"
#include "oi_misc.h"
#include "strutil.h"
#include "LogMacros.h"
#include "ObjectUpdatorMgr.h"
#include "GameTime.h"
#include "framework/GameObjectPool.h"
#include "framework/FightSvrMsgLayer.h"
#include "player/PlayerLogic.h"
#include "gamedata/GameDataMgr.h"
#include "dungeon/DungeonMgr.h"
#include "dungeon/DungeonLogic.h"
#include "module/luainterface/LuaBinding.h"
#include "module/luainterface/LuaDataAgt.h"
#include "dungeon/ChooseMgr.h"

extern unsigned char g_szMetalib_FightSvrCfg[];

CFightSvr::CFightSvr()
{
    m_dwIntervalPass = 0;
    m_ullLastUptTime = CGameTime::Instance().GetCurrSecond();
}

bool CFightSvr::AppInit()
{
    this->_InitGameObjPool();
    this->_InitGameObjUpdator();

	if (!FightSvrMsgLayer::Instance().Init())
	{
		LOGERR("FightSvrMsgLayer init error.");
		return false;
	}

	if (!CGameDataMgr::Instance().Init())
	{
		LOGERR("CGameDataMgr init error.");
		return false;
	}

	if (!DungeonMgr::Instance().Init())
	{
		LOGERR("DungeonMgr init error.");
		return false;
	}

    if (!ChooseMgr::Instance().Init())
    {
        LOGERR("ChooseMgr init error");
        return false;
    }

	// ?????????
#if 0
	Dungeon testDungeon;

	Troop testTroop;
	FightPlayer fightPlayer;
	fightPlayer.m_chGroup = PLAYER_GROUP_DOWN;
	PKGMETA::DT_TROOP_INFO stTInfo;
	bzero(&stTInfo, sizeof(PKGMETA::DT_TROOP_INFO));
	stTInfo.m_bId = 2;
	stTInfo.m_stGeneralInfo.m_dwId = 2;
	testTroop.Init(&testDungeon, &fightPlayer, stTInfo);

	Troop testTroopE;
	FightPlayer fightPlayerE;
	fightPlayerE.m_chGroup = PLAYER_GROUP_UP;
	PKGMETA::DT_TROOP_INFO stTInfoE;
	bzero(&stTInfoE, sizeof(PKGMETA::DT_TROOP_INFO));
	stTInfoE.m_bId = 1;
	stTInfoE.m_stGeneralInfo.m_dwId = 1;
	testTroopE.Init(&testDungeon, &fightPlayerE, stTInfoE);

	testTroop.m_oBuffManager.AddBuff(1011, &testTroopE, NULL);
	testTroop.m_oBuffManager.AddBuff(1021, &testTroopE, NULL);
	testTroop.m_oBuffManager.AddBuff(1031, &testTroopE, NULL);
	testTroop.m_oBuffManager.AddBuff(1041, &testTroopE, NULL);
	testTroop.m_oBuffManager.AddBuff(1051, &testTroopE, NULL);
	testTroop.m_oBuffManager.AddBuff(1061, &testTroopE, NULL);
	testTroop.m_oBuffManager.AddBuff(1071, &testTroopE, NULL);
	testTroop.m_oBuffManager.AddBuff(1081, &testTroopE, NULL);
	testTroop.m_oBuffManager.AddBuff(1091, &testTroopE, NULL);
	testTroop.m_oBuffManager.AddBuff(1101, &testTroopE, NULL);
	testTroop.m_oBuffManager.AddBuff(1111, &testTroopE, NULL);
	testTroop.m_oBuffManager.AddBuff(1121, &testTroopE, NULL);
	testTroop.m_oBuffManager.AddBuff(1131, &testTroopE, NULL);
	testTroop.m_oBuffManager.AddBuff(1141, &testTroopE, NULL);
	testTroop.m_oBuffManager.AddBuff(1151, &testTroopE, NULL);
	testTroop.m_oBuffManager.AddBuff(1161, &testTroopE, NULL);
	testTroop.m_oBuffManager.AddBuff(1171, &testTroopE, NULL);
	testTroop.m_oBuffManager.AddBuff(11, &testTroopE, NULL);
	testTroop.m_oBuffManager.AddBuff(12, &testTroopE, NULL);
	testTroop.m_oBuffManager.AddBuff(14, &testTroopE, NULL);
	testTroop.m_oBuffManager.AddBuff(15, &testTroopE, NULL);
	testTroop.m_oBuffManager.AddBuff(16, &testTroopE, NULL);
	testTroop.m_oBuffManager.AddBuff(20, &testTroopE, NULL);
	testTroop.m_oBuffManager.AddBuff(21, &testTroopE, NULL);
	testTroop.m_oBuffManager.AddBuff(22, &testTroopE, NULL);
	testTroop.m_oBuffManager.AddBuff(23, &testTroopE, NULL);
	testTroop.m_oBuffManager.AddBuff(24, &testTroopE, NULL); // ??????
	testTroop.m_oBuffManager.AddBuff(25, &testTroopE, NULL);
	testTroop.m_oBuffManager.AddBuff(26, &testTroopE, NULL);
	testTroop.m_oBuffManager.AddBuff(2, &testTroopE, NULL);
	testTroop.m_oBuffManager.AddBuff(4, &testTroopE, NULL);
	testTroop.m_oBuffManager.AddBuff(5004, &testTroopE, NULL);
	testTroop.m_oBuffManager.AddBuff(5, &testTroopE, NULL);
	testTroop.m_oBuffManager.AddBuff(6017, &testTroopE, NULL);
	testTroop.m_oBuffManager.AddBuff(6019, &testTroopE, NULL);
	testTroop.m_oBuffManager.AddBuff(9999, &testTroopE, NULL);

	int bef = 1000;
	int aft = 800;
	testTroop.ChgHp(VALUE_CHG_TYPE::CHG_HP_ATK_NORMAL, 0, &testTroopE, 0, bef, aft);
	testTroop.ChgHp(VALUE_CHG_TYPE::CHG_HP_GENERALSKILL, 1, &testTroopE, 0, bef, aft);

	testTroop.m_oBuffManager.AddBuff(24, NULL, NULL); // ??????
#endif

    return true;
}

void CFightSvr::AppFini()
{
    // ??ClusterGate
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_CLUSTER_SERV_STAT_NTF;
	m_stSsPkg.m_stHead.m_ullReservId = 0;
    m_stSsPkg.m_stBody.m_stClusterServStatNtf.m_stServInfo.m_bOnline = 0;
    m_stSsPkg.m_stBody.m_stClusterServStatNtf.m_stServInfo.m_iProcId = m_stConfig.m_iProcID;
    FightSvrMsgLayer::Instance().SendToClusterGate(m_stSsPkg);

    CAppFrame::AppFini();
}

void CFightSvr::AppUpdate()
{
    bool bIdle = true;

    // deal pkgs
    if (FightSvrMsgLayer::Instance().DealPkg() > 0)
    {
        bIdle = false;
    }

    ObjectUpdatorMgr::Instance().Update(bIdle);

    ChooseMgr::Instance().Update();

    this->_NotifyToCluster();

    if(bIdle)
    {
        MsSleep(1);
    }
}

void CFightSvr::_NotifyToCluster()
{
    m_dwIntervalPass = (uint32_t)(CGameTime::Instance().GetCurrSecond() - m_ullLastUptTime);
    if (m_dwIntervalPass >= m_stConfig.m_dwReportInterval)
    {
        m_ullLastUptTime = CGameTime::Instance().GetCurrSecond();
        m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_CLUSTER_SERV_STAT_NTF;
		m_stSsPkg.m_stHead.m_ullReservId = 0;
        m_stSsPkg.m_stBody.m_stClusterServStatNtf.m_stServInfo.m_bOnline = 1;
        m_stSsPkg.m_stBody.m_stClusterServStatNtf.m_stServInfo.m_dwLoad = (uint32_t)DungeonMgr::Instance().GetLoad();
        m_stSsPkg.m_stBody.m_stClusterServStatNtf.m_stServInfo.m_iProcId = m_stConfig.m_iProcID;
        m_stSsPkg.m_stBody.m_stClusterServStatNtf.m_stServInfo.m_iIp = m_stConfig.m_iConnSvrIp;
        m_stSsPkg.m_stBody.m_stClusterServStatNtf.m_stServInfo.m_wPort = m_stConfig.m_wConnSvrPort;
        FightSvrMsgLayer::Instance().SendToClusterGate(m_stSsPkg);
    }
}

void CFightSvr::_SetAppCfg()
{
    m_stAppCfg.m_pszBuff = (char*)&m_stConfig;
    m_stAppCfg.m_dwLen = sizeof(m_stConfig);
    m_stAppCfg.m_pstMetaLib = (LPTDRMETALIB)g_szMetalib_FightSvrCfg;
    StrCpy( m_stAppCfg.m_szMetaName, "FightSvrCfg", sizeof(m_stAppCfg.m_szMetaName) );
    snprintf( m_stAppCfg.m_szCfgFile, sizeof(m_stAppCfg.m_szCfgFile), "%s/config/%s", CWorkDir::string(), CConfXml::string() );
}

bool CFightSvr::_ReadCfg()
{
    if (!CAppFrame::_ReadCfg())
    {
        return false;
    }

    inet_aton( m_stConfig.m_szProcIDStr, (struct in_addr*)&m_stConfig.m_iProcID);
    inet_aton( m_stConfig.m_szConnIDStr, (struct in_addr*)&m_stConfig.m_iConnID);
    inet_aton( m_stConfig.m_szClusterGateIDStr, (struct in_addr*)&m_stConfig.m_iClusterGateID);
    inet_aton( m_stConfig.m_szConnSvrIpStr, (struct in_addr*)&m_stConfig.m_iConnSvrIp);
    return true;
}

void CFightSvr::_InitGameObjPool()
{
    GameObjectPool::Instance().SetMaxObjNum(GAMEOBJ_PLAYER, m_stConfig.m_iMaxFightControler);
	GameObjectPool::Instance().SetMaxObjNum(GAMEOBJ_DUNGEON, m_stConfig.m_iMaxDungeonNum);
}

void CFightSvr::_InitGameObjUpdator()
{
    ObjectUpdator* poObjUpdator = NULL;
    ObjectUpdatorMgr& roObjUpdatorMgr = ObjectUpdatorMgr::Instance();

    poObjUpdator = roObjUpdatorMgr.RegisterObjectUpdator( GAMEOBJ_PLAYER );
    poObjUpdator->BindCallback( UpdatePlayer );

    poObjUpdator = roObjUpdatorMgr.RegisterObjectUpdator( GAMEOBJ_DUNGEON, 30, 10, 300 );
    poObjUpdator->BindCallback( UpdateDungeon );
}

