#include "MessageSvr.h"
#include "GameTime.h"
#include "workdir.h"
#include "conf_xml.h"
#include "strutil.h"
#include "./framework/MessageSvrMsgLayer.h"
#include "LogMacros.h"
#include "TableDefine.h"
#include "mysql/MysqlHandler.h"
#include "module/MessageMgr.h"
#include "module/MessageInfo/DBWorkThreadMgr.h"
#include "./framework/MessageTransFrame.h"
extern unsigned char g_szMetalib_MessageSvrCfg[];

MessageSvr::MessageSvr()
{

}

bool MessageSvr::AppInit()
{
	if (!MessageSvrMsgLayer::Instance().Init())
	{
		return false;
	}

	if (!DBWorkThreadMgr::Instance().Init(&m_stConfig))
	{
		LOGERR_r("Init DBWorkThreadMgr falied");
		return false;
	}
	if ( !MessageTransFrame::Instance().Init(1000, 1))
	{
		LOGERR_r("MessageTransFrame Init failed");
		return false;
	}
	if (!MessageMgr::Instance().Init(&m_stConfig))
	{
		LOGERR_r("Init MessageMgr falied");
		return false;
	}

	return true;
}

void MessageSvr::AppFini()
{
	MessageMgr::Instance().Fini();
    MessageTransFrame::Instance().Fini();
    DBWorkThreadMgr::Instance().Fini();
	CAppFrame::AppFini();
}

void MessageSvr::AppUpdate()
{
	bool bIdle = true;

	if (MessageSvrMsgLayer::Instance().DealPkg() > 0)
	{
		bIdle = false;
	}
	MessageMgr::Instance().Update(bIdle);
	DBWorkThreadMgr::Instance().HandleDBThreadRsp();
	MessageTransFrame::Instance().Update();
	if (bIdle)
	{
		MsSleep(1);
	}
}

void MessageSvr::_SetAppCfg()
{
	m_stAppCfg.m_pszBuff = (char*) &m_stConfig;
	m_stAppCfg.m_dwLen = sizeof(m_stConfig);
	m_stAppCfg.m_pstMetaLib = (LPTDRMETALIB) g_szMetalib_MessageSvrCfg;
	StrCpy(m_stAppCfg.m_szMetaName, "MessageSvrCfg", sizeof(m_stAppCfg.m_szMetaName));
	snprintf(m_stAppCfg.m_szCfgFile, sizeof(m_stAppCfg.m_szCfgFile), "%s/config/MessageSvr.xml", CWorkDir::string());
}

bool MessageSvr::_ReadCfg()
{
	if (!CAppFrame::_ReadCfg())
	{
		return false;
	}
	
	inet_aton(m_stConfig.m_szProcIDStr, (struct in_addr*) &m_stConfig.m_iProcID);
	inet_aton(m_stConfig.m_szZoneSvrIDStr, (struct in_addr*) &m_stConfig.m_iZoneSvrID);
	
	return true;
}

