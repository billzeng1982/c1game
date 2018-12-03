#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "MysqlWorkThread.h"
#include "oi_misc.h"
#include "TableDefine.h"
#include "LogMacros.h"
#include "ThreadFrame.h"
#include "../module/BaseTable.h"
#include "../module/CommonTable.h"
#include "../module/RoleTable.h"
#include "../module/GuildTable.h"
#include "../module/FriendTable.h"
#include "../module/CombineMgr.h"


bool CMysqlWorkThread::_ThreadAppInit(void* pvAppData)
{
	ThreadInitPara* pstAppData = (ThreadInitPara*)pvAppData;
	m_pstConfig = pstAppData->m_pstCombineSvrCfg;

    // init mysql hanlder
    if( !m_oMysqlHandler.ConnectDB( m_pstConfig->m_szMysqlDBAddr,
                                    m_pstConfig->m_wMysqlPort,
                                    m_pstConfig->m_szMysqlDBName,
                                    m_pstConfig->m_szMysqlUser,
                                    m_pstConfig->m_szMysqlPassword ) )
    {
        LOGERR_r( "Connect mysql failed!" );
        return false;
    }

	if (strcmp(pstAppData->m_pstTableName, ROLE_TABLE_NAME) == 0)
	{
		m_pstBaseTable = new RoleTable;
	}
	else if (strcmp(pstAppData->m_pstTableName,  GUILD_TABLE_NAME) == 0)
	{
		m_pstBaseTable = new GuildTable;
	}
	else if (strcmp(pstAppData->m_pstTableName, FRIEND_TABLE_NAME) == 0)
	{
		m_pstBaseTable = new FriendTable;
	}
	else
	{
		m_pstBaseTable = new CommonTable;
	}
	if (!m_pstBaseTable)
	{
		LOGERR_r("CMysqlWorkThread::_ThreadAppInit: m_pstBaseTable is NULL! ");
		return false;
	}
	if (!m_pstBaseTable->Init(&m_oMysqlHandler, pstAppData->m_iTableSvrId, pstAppData->m_pstTableName, pstAppData->m_pstTableColumn))
	{
		return false;
	}
	m_bOnceRun = true;
    return true;
}

void CMysqlWorkThread::_ThreadAppFini()
{
	m_pstBaseTable->Fini();
	delete m_pstBaseTable;
	m_pstBaseTable = NULL;
}

int CMysqlWorkThread::_ThreadAppProc()
{
    int iRet = _CheckMysqlConn();
    if (iRet <= 0)
    {
        return -1;
    }
	MsSleep(200);
	if (m_bOnceRun)
	{
		m_bOnceRun = false;
		Work();

	}
    return -1;
}



int CMysqlWorkThread::_CheckMysqlConn()
{
    // check alive
    if (!m_oMysqlHandler.IsConnAlive())
    {
        if (!m_oMysqlHandler.ReconnectDB())
        {
            return -1; //change to idle
        }
    }

    time_t lCurTime = CGameTime::Instance().GetCurrSecond();
    if (lCurTime - m_oMysqlHandler.GetLastPingTime() >= m_pstConfig->m_iPingFreq)
    {
        // ping mysql
        m_oMysqlHandler.Ping();
        m_oMysqlHandler.SetLastPingTime(lCurTime);
    }

    return 1;
}


int CMysqlWorkThread::Work()
{
	int ret = m_pstBaseTable->Work();
	CombineMgr::Instance().ChangeState(m_iThreadIdx, ret);
	return 0;
}

