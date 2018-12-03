#include "FriendSvr.h"
#include "GameTime.h"
#include "workdir.h"
#include "conf_xml.h"
#include "strutil.h"
#include "LogMacros.h"
#include "TableDefine.h"
#include "framework/FriendSvrMsgLayer.h"
#include "module/FriendMgr.h"
#include "module/PlayerInfo/RedisWorkThreadMgr.h"
#include "module/FriendInfo/DBWorkThreadMgr.h"

extern unsigned char g_szMetalib_FriendSvrCfg[];

FriendSvr::FriendSvr()
{

}

bool FriendSvr::AppInit()
{
	if( !FriendSvrMsgLayer::Instance().Init() )
	{
		return false;
	}
	if (!FriendMgr::Instance().Init(&m_stConfig))
	{
		LOGERR_r("Init FriendMgr falied");
		return false;
	}
	if ( !FriendTransFrame::Instance().Init(1000, 1))
	{
		LOGERR_r("FriendTransFrame Init failed");
		return false;
	}
// 	if ( !m_oRedisHandler.Connect(m_stConfig.m_szRedisAddr, m_stConfig.m_wRedisPort))
// 	{
// 		LOGERR_r( "Connect redis failed!" );
// 		return false;
// 	}
	if ( !DBWorkThreadMgr::Instance().Init(&m_stConfig))
	{
		LOGERR_r( "Init DBWorkThread  failed!" );
		return false;
	}
	if ( !RedisWorkThreadMgr::Instance().Init(&m_stConfig))
	{
		LOGERR_r( "Init RedisWorkThread  failed!" );
		return false;
	}

	LOGRUN_r( "FriendSvr::AppInit OK! " );
	return true;
}

void FriendSvr::AppFini()
{
    
    FriendMgr::Instance().Fini();
    DBWorkThreadMgr::Instance().Fini();
    RedisWorkThreadMgr::Instance().Fini();
	CAppFrame::AppFini();
}

void FriendSvr::AppUpdate()
{
	bool bIdle = true;
	if (FriendSvrMsgLayer::Instance().DealPkg() > 0)
	{
		bIdle = false;
	}
	FriendMgr::Instance().Update(bIdle);
    FriendTransFrame::Instance().Update();
	DBWorkThreadMgr::Instance().HandleRspMsg();
	RedisWorkThreadMgr::Instance().HandleRspMsg();
	if (bIdle)
	{
		MsSleep(1);
	}
}

void FriendSvr::_SetAppCfg( )
{
	m_stAppCfg.m_pszBuff = (char*)&m_stConfig;
	m_stAppCfg.m_dwLen = sizeof(m_stConfig);
	m_stAppCfg.m_pstMetaLib = (LPTDRMETALIB)g_szMetalib_FriendSvrCfg;
	StrCpy( m_stAppCfg.m_szMetaName, "FriendSvrCfg", sizeof(m_stAppCfg.m_szMetaName) );
	snprintf( m_stAppCfg.m_szCfgFile, sizeof(m_stAppCfg.m_szCfgFile), "%s/config/FriendSvr.xml", CWorkDir::string());
}

bool FriendSvr::_ReadCfg( )
{
	if( !CAppFrame::_ReadCfg() )
	{
		return false;
	}

	inet_aton( m_stConfig.m_szProcIDStr, (struct in_addr* )&m_stConfig.m_iProcID );
	inet_aton( m_stConfig.m_szZoneSvrIDStr, (struct in_addr* )&m_stConfig.m_iZoneSvrID );

	return true;
}

