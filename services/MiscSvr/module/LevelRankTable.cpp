#include "LevelRankTable.h"
#include "LogMacros.h"

#define MARK_ID 0
#define BLOB_NAME_LEVEL_RANK "LevelRecordBlob"

using namespace PKGMETA;

int Cmp(const void *pstFirst, const void *pstSecond)
{
	DT_FIGHT_LEVEL_RECORD_BIG_ITEM* pstItemFirst  = (DT_FIGHT_LEVEL_RECORD_BIG_ITEM*)pstFirst;
	DT_FIGHT_LEVEL_RECORD_BIG_ITEM* pstItemSecond = (DT_FIGHT_LEVEL_RECORD_BIG_ITEM*)pstSecond;

	int iResult = (int)pstItemFirst->m_dwLevelId - (int)pstItemSecond->m_dwLevelId;
	return iResult;
}

LevelRankTable::LevelRankTable()
{

}

LevelRankTable::~LevelRankTable()
{

}


bool LevelRankTable::Init(MISCSVRCFG* pstConfig)
{
	LOGRUN_r("Init LevelRankTableMgr module");
	m_pstConfig = pstConfig;

	if (NULL == m_pstConfig)
	{
		LOGERR_r("pstconfig is null");
		return false;
	}

	if (!_ConnectToDB())
	{
		LOGERR_r( "Connect mysql failed!" );
		return false;
	}

	return true;
}



bool LevelRankTable::_MakeFirstData(PKGMETA::SS_PKG_LEVEL_RECORD_READ_RSP& rstLevelRecordBlob)
{
	uint64_t ullUseSize = 0;
	DT_FIGHT_LEVEL_RECORD_INFO stRecordInfo;

	stRecordInfo.m_iCount = 0;
         
	int iRet = stRecordInfo.pack((char*)rstLevelRecordBlob.m_szData,  MAX_LEN_LEVEL_RECORD, &ullUseSize);
	if( iRet != TdrError::TDR_NO_ERROR)
	{
		LOGERR_r("pack rstLevelRecordBlob failed, Ret=%d", iRet);
		return false;
	}
	rstLevelRecordBlob.m_dwLen = (int)ullUseSize;
	return true;
}

int LevelRankTable::_InsertFirstData(PKGMETA::SS_PKG_LEVEL_RECORD_READ_RSP& rstLevelRecordBlob)
{
	//int iTableNum = m_pstConfig->m_wSvrId;
	char bufToDB[LEVEL_RANK_INFO_BINBUF_SIZE];
    
	bufToDB[0] = '\0';
	if (!m_stMysqlHandler.BinToStr((char*)rstLevelRecordBlob.m_szData, rstLevelRecordBlob.m_dwLen, bufToDB, LEVEL_RANK_INFO_BINBUF_SIZE))
	{
		LOGERR_r("convert to mysql bin failed, LevelRank");
		return ERR_SYS;
	}
	m_stMysqlHandler.FormatSql( "INSERT INTO tbl_level_record SET "
		"LevelRecordBlob='%s',MarkId=%d",  bufToDB, MARK_ID);

	if( m_stMysqlHandler.Execute( ) < 0 )
	{
		LOGERR_r("excute sql failed: %s", m_stMysqlHandler.GetLastErrMsg() );
		return ERR_DB_ERROR;
	}
	return ERR_NONE;
}

int LevelRankTable::UpdateFromDB(PKGMETA::SS_PKG_LEVEL_RECORD_READ_RSP& rstLevelRecordBlob)
{
	LOGERR_r("In LevelRankTableMgr::UpdateFromDB");
	_CheckMysqlConn();
	//int iTableNum = m_pstConfig->m_wSvrId;
	m_stMysqlHandler.FormatSql(
		"SELECT  "
		BLOB_NAME_LEVEL_RANK /*1*/
		" FROM tbl_level_record WHERE MarkId=%d ",
		MARK_ID );
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
		_MakeFirstData(rstLevelRecordBlob);
		_InsertFirstData(rstLevelRecordBlob);
		return ERR_NONE;
	}

	CMysqlRowIter& oRowIter = m_stMysqlHandler.GetRowIter();
	oRowIter.Begin( m_stMysqlHandler.GetResult() );
	rstLevelRecordBlob.m_dwLen = oRowIter.GetField(0)->GetBinData( (char*)rstLevelRecordBlob.m_szData, MAX_LEN_LEVEL_RECORD);
	return ERR_NONE;
}

int LevelRankTable::UpdateToDB(PKGMETA::SS_PKG_LEVEL_RECORD_SAVE_REQ& rstLevelRecordBlob)
{
	//LOGERR_r("In LevelRankTableMgr::UpdateToDB");
	_CheckMysqlConn();
	//int iTableNum = m_pstConfig->m_wSvrId;
	char bufToDB[LEVEL_RANK_INFO_BINBUF_SIZE];
	bufToDB[0] = '\0';
	if (!m_stMysqlHandler.BinToStr((char*)rstLevelRecordBlob.m_szData, rstLevelRecordBlob.m_dwLen, bufToDB, LEVEL_RANK_INFO_BINBUF_SIZE))
	{
		LOGERR_r("convert to mysql bin failed, LevelRank");
		return ERR_SYS;
	}
	m_stMysqlHandler.FormatSql( "UPDATE tbl_level_record SET "
		"LevelRecordBlob='%s' "
		"where MarkId='%d'", bufToDB, MARK_ID );

	if( m_stMysqlHandler.Execute( ) < 0 )
	{
		LOGERR_r("excute sql failed: %s", m_stMysqlHandler.GetLastErrMsg() );
		return ERR_DB_ERROR;
	}
	return ERR_NONE;
}

int LevelRankTable::_ConnectToDB()
{
	return m_stMysqlHandler.ConnectDB(m_pstConfig->m_szDBAddr, m_pstConfig->m_wPort, m_pstConfig->m_szDBName, 
		m_pstConfig->m_szUser, m_pstConfig->m_szPassword);
}

int LevelRankTable::_CheckMysqlConn()
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
