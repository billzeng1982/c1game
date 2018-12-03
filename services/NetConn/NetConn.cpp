#include "NetConn.h"
#include "workdir.h"
#include "conf_xml.h"
#include "GameTime.h"
#include "oi_misc.h"
#include "comm_func.h"
#include "og_comm.h"
#include "strutil.h"
#include "cs_proto.h"
#include "ConnBase.h"
#include "TCP/ConnTCP.h"
#include "UDP/ConnUDP.h"

using namespace PKGMETA;

extern unsigned char g_szMetalib_NetConnCfg[];

CNetConn::CNetConn()
{
    m_poConnCom = NULL;
}

CNetConn::~CNetConn()
{
    if (m_poConnCom != NULL)
    {
        delete(m_poConnCom);
        m_poConnCom = NULL;
    }
}

bool CNetConn::AppInit()
{
    if (m_stConfig.m_stTcpConfig.m_iTCPSwitch)
    {
        m_poConnCom =new CConnTCP(&m_stConfig);
    }
    else if (m_stConfig.m_stUdpConfig.m_iUDPSwitch)
    {
        m_poConnCom =new CConnUDP(&m_stConfig);
    }
    else
    {
        LOGERR("init connCom failed.");
        return false;
    }

    if (!m_poConnCom)
    {
        LOGERR("alloc connCom failed.");
        return false;
    }

    if( !m_poConnCom->Create() )
    {
        return false;
    }

    return true;
}

void CNetConn::AppUpdate()
{
    int  iHndCltNum = 0;
    int  iHndBusNum = 0;

    iHndCltNum = m_poConnCom->HandleNetIO();
    iHndBusNum = m_poConnCom->HandleBusMsg();

    // 检测client连线是否正常 
    bool bIdle = true;
    if (iHndCltNum>0 || iHndBusNum>0)
    { 
        bIdle = false;
    }
    else
    {
        bIdle = true; 
    }

    m_poConnCom->Update(bIdle);

    //m_poConnCom->LogStatsInfo();

    if (bIdle)
    {
        MsSleep(1); 
    }
}

void CNetConn::_SetAppCfg( )
{
    m_stAppCfg.m_pszBuff = (char*)&m_stConfig;
    m_stAppCfg.m_dwLen = sizeof(m_stConfig);
    m_stAppCfg.m_pstMetaLib = (LPTDRMETALIB)g_szMetalib_NetConnCfg;
    StrCpy( m_stAppCfg.m_szMetaName, "NetConnCfg", sizeof(m_stAppCfg.m_szMetaName) );
    snprintf( m_stAppCfg.m_szCfgFile, sizeof(m_stAppCfg.m_szCfgFile), "%s/config/%s", CWorkDir::string(), CConfXml::string() );
}

bool CNetConn::_ReadCfg()
{
    if( !CAppFrame::_ReadCfg() )
    {
        return false;
    }

    inet_aton( m_stConfig.m_szProcIDStr, (struct in_addr*)&m_stConfig.m_iProcID );
    inet_aton( m_stConfig.m_stConnSvrInfo.m_szLogicSvrIDStr, (struct in_addr*)&m_stConfig.m_stConnSvrInfo.m_iLogicSvrID );

    return true;
}

