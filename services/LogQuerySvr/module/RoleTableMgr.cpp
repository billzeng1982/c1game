#include "RoleTableMgr.h"
#include "LogMacros.h"


RoleTableMgr::RoleTableMgr()
{

}

RoleTableMgr::~RoleTableMgr()
{

}

bool RoleTableMgr::Init(LOGQUERYSVRCFG* pstConfig)
{
	m_pstConfig = pstConfig;

	if (NULL == m_pstConfig)
	{
		LOGERR("pstconfig is null");
		return false;
	}

	//先连接mysql数据库
	if (!m_stMysqlHandler.ConnectDB(m_pstConfig->m_szDBAddr, m_pstConfig->m_wPort, m_pstConfig->m_szDBName, 
		m_pstConfig->m_szUser, m_pstConfig->m_szPassword))
	{
		LOGERR("connect to mysql failed!");
		return false;
	}

	return true;
}

int RoleTableMgr::GetRoleMajestyInfo(uint64_t ullUin, OUT PKGMETA::DT_ROLE_MAJESTY_BLOB& rstMajestyBlob)
{
	int iRet = _CheckMysqlConn();
	if (iRet < 0)
	{
		return -1;
	}

	int iTableNum = GET_TABLE_NUM(ullUin);
	m_stMysqlHandler.FormatSql(
		"SELECT  "
		BLOB_NAME_ROLE_MAJESTY /*1*/
		" FROM tbl_role WHERE Uin=%lu ",
		ullUin );
	if( m_stMysqlHandler.Execute( ) < 0 )
	{
		LOGERR_r("excute sql failed: %s", m_stMysqlHandler.GetLastErrMsg() );
		return ERR_DB_ERROR;
	}

	int iEffectRow = m_stMysqlHandler.StoreResult();
	if( iEffectRow < 0 )
	{
		return ERR_DB_ERROR;
	}
	if( 0 == iEffectRow )
	{
		return ERR_NOT_FOUND;
	}

	CMysqlRowIter& oRowIter = m_stMysqlHandler.GetRowIter();
	oRowIter.Begin( m_stMysqlHandler.GetResult() );
	rstMajestyBlob.m_iLen = oRowIter.GetField(0)->GetBinData( (char*)rstMajestyBlob.m_szData, MAX_LEN_ROLE_MAJESTY);
	
	return ERR_NONE;
}

int RoleTableMgr::GetRoleGuildInfo(uint64_t ullUin, OUT PKGMETA::DT_ROLE_GUILD_BLOB& rstGuildBlob)
{
	int iRet = _CheckMysqlConn();

	if (iRet < 0)
	{
		return -1;
	}

	int iTableNum = GET_TABLE_NUM(ullUin);
	m_stMysqlHandler.FormatSql(
		"SELECT  "
		BLOB_NAME_ROLE_GUILD /*1*/
		" FROM tbl_role WHERE Uin=%lu ",
		ullUin );
	if( m_stMysqlHandler.Execute( ) < 0 )
	{
		LOGERR_r("excute sql failed: %s", m_stMysqlHandler.GetLastErrMsg() );
		return ERR_DB_ERROR;
	}

	int iEffectRow = m_stMysqlHandler.StoreResult();
	if( iEffectRow < 0 )
	{
		return ERR_DB_ERROR;
	}
	if( 0 == iEffectRow )
	{
		return ERR_NOT_FOUND;
	}

	CMysqlRowIter& oRowIter = m_stMysqlHandler.GetRowIter();
	oRowIter.Begin( m_stMysqlHandler.GetResult() );
	rstGuildBlob.m_iLen = oRowIter.GetField(0)->GetBinData( (char*)rstGuildBlob.m_szData, MAX_LEN_ROLE_GUILD);
	
	return ERR_NONE;
}

int RoleTableMgr::GetRoleGcardInfo(uint64_t ullUin, OUT PKGMETA::DT_ROLE_GCARD_BLOB& rstGcardBlob)
{
	int iRet = _CheckMysqlConn();

	if (iRet < 0)
	{
		return -1;
	}

	int iTableNum = GET_TABLE_NUM(ullUin);
	m_stMysqlHandler.FormatSql(
		"SELECT  "
		BLOB_NAME_ROLE_GCARD /*1*/
		" FROM tbl_role WHERE Uin=%lu ",
		ullUin );
	if( m_stMysqlHandler.Execute( ) < 0 )
	{
		LOGERR_r("excute sql failed: %s", m_stMysqlHandler.GetLastErrMsg() );
		return ERR_DB_ERROR;
	}

	int iEffectRow = m_stMysqlHandler.StoreResult();
	if( iEffectRow < 0 )
	{
		return ERR_DB_ERROR;
	}
	if( 0 == iEffectRow )
	{
		return ERR_NOT_FOUND;
	}

	CMysqlRowIter& oRowIter = m_stMysqlHandler.GetRowIter();
	oRowIter.Begin( m_stMysqlHandler.GetResult() );
	rstGcardBlob.m_iLen = oRowIter.GetField(0)->GetBinData( (char*)rstGcardBlob.m_szData, MAX_LEN_ROLE_GCARD);

	return ERR_NONE;
}

int RoleTableMgr::_CheckMysqlConn()
{
	// check alive
	if (!m_stMysqlHandler.IsConnAlive())
	{
		if (!m_stMysqlHandler.ReconnectDB())
		{
			return -1; //change to idle
		}
	}

	time_t lCurTime = CGameTime::Instance().GetCurrSecond();
	if (lCurTime - m_stMysqlHandler.GetLastPingTime() >= m_pstConfig->m_iPingFreq)
	{
		// ping mysql
		m_stMysqlHandler.Ping();
		m_stMysqlHandler.SetLastPingTime(lCurTime);
	}

	return 1;
}

int RoleTableMgr::GetRoleUinByName(const char* szName, uint64_t& ullUin)
{
	int iRet = _CheckMysqlConn();

	if (iRet < 0)
	{
		return -1;
	}

	int iTableNum = _GetSvrId();
	m_stMysqlHandler.FormatSql(
		"SELECT Uin FROM tbl_role WHERE RoleName=\'%s\'",
		szName );
	if( m_stMysqlHandler.Execute( ) < 0 )
	{
		LOGERR_r("excute sql failed: %s", m_stMysqlHandler.GetLastErrMsg() );
		return ERR_DB_ERROR;
	}

	int iEffectRow = m_stMysqlHandler.StoreResult();
	if( iEffectRow < 0 )
	{
		return ERR_DB_ERROR;
	}
	if( 0 == iEffectRow )
	{
		return ERR_NOT_FOUND;
	}

	CMysqlRowIter& oRowIter = m_stMysqlHandler.GetRowIter();
	oRowIter.Begin( m_stMysqlHandler.GetResult());
	ullUin = (uint64_t)oRowIter.GetField(0)->GetBigInteger();

	return ERR_NONE;
}

int RoleTableMgr::GetChannelName(uint64_t ullUin, OUT char* szChannelName, size_t size)
{
    int iRet = _CheckMysqlConn();

    if (iRet < 0)
    {
        return -1;
    }

    int iTableNum = GET_TABLE_NUM(ullUin);
    m_stMysqlHandler.FormatSql(
        "SELECT ChannelID FROM tbl_account_%d WHERE Uin=%lu",
        iTableNum, ullUin );
    if( m_stMysqlHandler.Execute( ) < 0 )
    {
        LOGERR_r("excute sql failed: %s", m_stMysqlHandler.GetLastErrMsg() );
        return ERR_DB_ERROR;
    }

    int iEffectRow = m_stMysqlHandler.StoreResult();
    if( iEffectRow < 0 )
    {
        return ERR_DB_ERROR;
    }
    if( 0 == iEffectRow )
    {
        return ERR_NOT_FOUND;
    }

    CMysqlRowIter& oRowIter = m_stMysqlHandler.GetRowIter();
    oRowIter.Begin( m_stMysqlHandler.GetResult() );
    snprintf(szChannelName, size, "%s", oRowIter.GetField(0)->GetString());

    return ERR_NONE;
}
