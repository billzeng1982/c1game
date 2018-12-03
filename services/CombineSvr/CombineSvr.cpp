#include "CombineSvr.h"
#include "GameTime.h"
#include "workdir.h"
#include "conf_xml.h"
#include "strutil.h"
#include "LogMacros.h"
#include "./module/CombineMgr.h"


extern unsigned char g_szMetalib_CombineSvrCfg[];

CombineSvr::CombineSvr()
{

}

bool CombineSvr::AppInit()
{
	if (!CombineMgr::Instance().Init(&m_stConfig))
	{
		return false;
	}




	return true;
}

void CombineSvr::AppFini()
{
	CombineMgr::Instance().Fini();
	CAppFrame::AppFini();
}

void CombineSvr::AppUpdate()
{
	bool bIdle = true;

	if (bIdle)
	{
		MsSleep(1);
	}
	CombineMgr::Instance().Update();
}

void CombineSvr::_SetAppCfg()
{
	m_stAppCfg.m_pszBuff = (char*) &m_stConfig;
	m_stAppCfg.m_dwLen = sizeof(m_stConfig);
	m_stAppCfg.m_pstMetaLib = (LPTDRMETALIB) g_szMetalib_CombineSvrCfg;
	StrCpy(m_stAppCfg.m_szMetaName, "CombineSvrCfg", sizeof(m_stAppCfg.m_szMetaName));
	snprintf(m_stAppCfg.m_szCfgFile, sizeof(m_stAppCfg.m_szCfgFile), "./CombineSvr.xml");
}

bool CombineSvr::_ReadCfg()
{
	if (!CAppFrame::_ReadCfg())
	{
		return false;
	}
	return true;
}

