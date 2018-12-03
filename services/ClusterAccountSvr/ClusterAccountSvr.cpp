#include "ClusterAccountSvr.h"
#include "GameTime.h"
#include "workdir.h"
#include "conf_xml.h"
#include "strutil.h"
#include "ClusterAccSvrMsgLayer.h"
#include "ClusterAccTable.h"
#include "LogMacros.h"
#include "TableDefine.h"

extern unsigned char g_szMetalib_ClusterAccountSvrCfg[];

ClusterAccountSvr::ClusterAccountSvr() {
	m_astWorkThreads = NULL;
}

bool ClusterAccountSvr::AppInit() {
	if (!ClusterAccSvrMsgLayer::Instance().Init()) 
    {
		return false;
	}

	if (m_stConfig.m_iWorkThreadNum <= 0) 
    {
		assert(false);
		return false;
	}

	m_astWorkThreads = new CDBWorkThread[m_stConfig.m_iWorkThreadNum];
	if (!m_astWorkThreads) 
    {
		return false;
	}

	key_t iShmKey = m_stConfig.m_iThreadQBaseShmKey;
	bool bRet = 0;
    for (int i = 0; i<m_stConfig.m_iWorkThreadNum; i++) 
    {
        bRet = m_astWorkThreads[i].InitThread(i, m_stConfig.m_dwThreadQSize, THREAD_QUEUE_DUPLEX, (void*)&m_stConfig, &iShmKey);
		if (!bRet) 
        {
			LOGERR_r("Init thread <%d> failed", i);
			return false;
		}
	}

	return true;
}

void ClusterAccountSvr::AppFini() 
{
    for (int i = 0; i < m_stConfig.m_iWorkThreadNum; i++) {
		m_astWorkThreads[i].FiniThread();
    }
    CAppFrame::AppFini();
}

void ClusterAccountSvr::AppUpdate() {
	bool bIdle = true;

	if (ClusterAccSvrMsgLayer::Instance().DealPkg() > 0) {
		bIdle = false;
	}

	if (bIdle) {
		MsSleep(1);
	}
}

void ClusterAccountSvr::_SetAppCfg() {
	m_stAppCfg.m_pszBuff = (char*) &m_stConfig;
	m_stAppCfg.m_dwLen = sizeof(m_stConfig);
	m_stAppCfg.m_pstMetaLib = (LPTDRMETALIB) g_szMetalib_ClusterAccountSvrCfg;
	StrCpy(m_stAppCfg.m_szMetaName, "ClusterAccountSvrCfg",
			sizeof(m_stAppCfg.m_szMetaName));
	snprintf(m_stAppCfg.m_szCfgFile, sizeof(m_stAppCfg.m_szCfgFile),
			"%s/config/ClusterAccountSvr.xml", CWorkDir::string());
}

bool ClusterAccountSvr::_ReadCfg() {
	if (!CAppFrame::_ReadCfg()) {
		return false;
	}

	inet_aton(m_stConfig.m_szProcIDStr,
			(struct in_addr*) &m_stConfig.m_iProcID);
	inet_aton(m_stConfig.m_szClusterGateIDStr,
			(struct in_addr*) &m_stConfig.m_iClusterGateID);

	return true;
}

