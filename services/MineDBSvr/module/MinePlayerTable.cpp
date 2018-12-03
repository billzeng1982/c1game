
#include "MinePlayerTable.h"
#include "LogMacros.h"
#include "comm_func.h"
#include "strutil.h"
#include "TableDefine.h"
#include "../MineDBSvr.h"
#include "../thread/DBWorkThread.h"


using namespace PKGMETA;

MinePlayerTable::MinePlayerTable()
{
    m_szBinBuf = NULL;
}

MinePlayerTable::~MinePlayerTable()
{
    m_szBinBuf = NULL;
}

bool MinePlayerTable::Init(CMysqlHandler* poMysqlHandler, MINEDBSVRCFG* pstConfig, char* szBinBuff)
{
    if (NULL == pstConfig || szBinBuff == NULL)
    {
        return false;
    }
    StrCpy(m_szTableName, MINE_PLAYER_TABLE_NAME, sizeof(m_szTableName));
    m_pstConfig = pstConfig;
    m_szBinBuf = szBinBuff;
    m_stMysqlHandler.ConnectDB(m_pstConfig->m_szDBAddr, m_pstConfig->m_wPort, m_pstConfig->m_szDBName,
        m_pstConfig->m_szUser, m_pstConfig->m_szPassword);
    return true;
}



int MinePlayerTable::UptData(PKGMETA::DT_MINE_PLAYER_DATA& rstData)
{
    char* pszSql = m_stMysqlHandler.GetSql();
    pszSql[0] = '\0';
    //int iTableNum = GET_TABLE_NUM(rstData.m_ullUin);
    //replace into test_tbl(id, dr) values(1, '2'), (2, '3'), ...(x, 'y');
    if (!m_stMysqlHandler.BinToStr((char*)rstData.m_stPlayerBlob.m_szData, rstData.m_stPlayerBlob.m_iLen, m_szBinBuf, CDBWorkThread::CONST_MINE_ORE_INFO_BINBUF_SIZE))
    {
        LOGERR_r("PlayerBlob convert to mysql bin failed, Uid<%lu> Version<%u>", rstData.m_ullUin, rstData.m_dwVersion);
        return ERR_DB_ERROR;
    }
    StrCat(pszSql, DB_SQL_STR_SIZE, " REPLACE INTO %s (Uin, Version, PlayerBlob) VALUES (%lu, %u, '%s')", 
        m_szTableName, rstData.m_ullUin, rstData.m_dwVersion, m_szBinBuf);
    if (m_stMysqlHandler.Execute(pszSql) < 0)
    {
        LOGERR_r("excute sql failed: %s", m_stMysqlHandler.GetLastErrMsg());
        return ERR_DB_ERROR;
    }
    return ERR_NONE;
}

int MinePlayerTable::GetData(uint64_t ullKey, OUT PKGMETA::DT_MINE_PLAYER_DATA& rstData)
{
    //int iTableNum = GET_TABLE_NUM(ullKey);
    //Select * From 表5 Where 名字 in('陈杰', '陈', '王洁', '王兵')
    char* pszSql = m_stMysqlHandler.GetSql();
    pszSql[0] = '\0';
    StrCat(pszSql, DB_SQL_STR_SIZE, " SELECT Uin, Version, PlayerBlob FROM %s WHERE Uin in ( %lu ) ", 
        m_szTableName, ullKey);

    if (m_stMysqlHandler.Execute(pszSql) < 0)
    {
        LOGERR_r("excute sql failed: %s", m_stMysqlHandler.GetLastErrMsg());
        return ERR_DB_ERROR;
    }
    int iEffectRow = m_stMysqlHandler.StoreResult();
    if (iEffectRow < 0)
    {
        return ERR_DB_ERROR;
    }
    if (0 == iEffectRow)
    {
        return ERR_NOT_FOUND;
    }
    CMysqlRowIter& oRowIter = m_stMysqlHandler.GetRowIter();
	oRowIter.Begin(m_stMysqlHandler.GetResult());
    int i = 0;
    rstData.m_ullUin = (uint64_t)oRowIter.GetField(i++)->GetBigInteger();
    rstData.m_dwVersion = (uint32_t)oRowIter.GetField(i++)->GetInteger();
    rstData.m_stPlayerBlob.m_iLen = oRowIter.GetField(i++)->GetBinData((char*)rstData.m_stPlayerBlob.m_szData, sizeof(rstData.m_stPlayerBlob.m_szData));

    return ERR_NONE;

}

