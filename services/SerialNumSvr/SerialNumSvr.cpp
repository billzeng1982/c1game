#include "SerialNumSvr.h"
#include "GameTime.h"
#include "workdir.h"
#include "conf_xml.h"
#include "strutil.h"
#include "./framework/SerialNumSvrMsgLayer.h"
#include "LogMacros.h"
#include "TableDefine.h"
#include "GameDataMgr.h"

extern unsigned char g_szMetalib_SerialNumSvrCfg[];

SerialNumSvr::SerialNumSvr()
{

}

bool SerialNumSvr::AppInit()
{
	if (!SerialNumSvrMsgLayer::Instance().Init())
	{
        LOGERR_r("SerialNumSvrMsgLayer Init failed");
		return false;
	}

	if (!CGameDataMgr::Instance().Init())
	{
        LOGERR_r("GameDataMgr Init failed");
		return false;
	}

	if (m_stConfig.m_iRedisWorkThreadNum <= 0)
	{
		return false;
	}

	m_astWorkThreads = new RedisWorkThread[m_stConfig.m_iRedisWorkThreadNum];
	if (!m_astWorkThreads)
	{
		return false;
	}

	key_t iShmKey = m_stConfig.m_iRedisThreadQBaseShmKey;
	bool bRet = 0;
	for (int i = 0; i<m_stConfig.m_iRedisWorkThreadNum; i++)
	{
		bRet = m_astWorkThreads[i].InitThread(i, m_stConfig.m_dwRedisThreadQSize, THREAD_QUEUE_DUPLEX, (void*)&m_stConfig, &iShmKey);
		if (!bRet)
		{
			LOGERR_r("Init thread <%d> failed", i);
			return false;
		}
	}

	return true;
}

void SerialNumSvr::AppFini()
{
	for (int i = 0; i < m_stConfig.m_iRedisWorkThreadNum; i++)
	{
		m_astWorkThreads[i].FiniThread();
	}

	CAppFrame::AppFini();
}

void SerialNumSvr::AppUpdate()
{
	bool bIdle = true;

	if (SerialNumSvrMsgLayer::Instance().DealPkg() > 0)
	{
		bIdle = false;
	}

	if (bIdle)
	{
		MsSleep(1);
	}
}

void SerialNumSvr::_SetAppCfg()
{
	m_stAppCfg.m_pszBuff = (char*) &m_stConfig;
	m_stAppCfg.m_dwLen = sizeof(m_stConfig);
	m_stAppCfg.m_pstMetaLib = (LPTDRMETALIB) g_szMetalib_SerialNumSvrCfg;
	StrCpy(m_stAppCfg.m_szMetaName, "SerialNumSvrCfg", sizeof(m_stAppCfg.m_szMetaName));
	snprintf(m_stAppCfg.m_szCfgFile, sizeof(m_stAppCfg.m_szCfgFile), "%s/config/SerialNumSvr.xml", CWorkDir::string());
}

bool SerialNumSvr::_ReadCfg()
{
	if (!CAppFrame::_ReadCfg())
	{
		return false;
	}

	inet_aton(m_stConfig.m_szProcIDStr, (struct in_addr*) &m_stConfig.m_iProcID);
	inet_aton(m_stConfig.m_szClusterGateIDStr, (struct in_addr*) &m_stConfig.m_iClusterGateID);

	return true;
}

