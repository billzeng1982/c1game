
#include "LogMacros.h"
#include "comm_func.h"
#include "strutil.h"
#include "TableDefine.h"
#include "./MineOreTable.h"
#include "../MineDBSvr.h"
#include "../thread/DBWorkThread.h"


using namespace PKGMETA;



MineOreTable::MineOreTable()
{
    m_szBinBuf = NULL;
}

MineOreTable::~MineOreTable()
{
    m_szBinBuf = NULL;
}

bool MineOreTable::Init(CMysqlHandler* poMysqlHandler, MINEDBSVRCFG* pstConfig, char* szBinBuff)
{
    if (NULL == pstConfig || szBinBuff == NULL)
    {
        return false;
    }
    StrCpy(m_szTableName, MINE_ORE_TABLE_NAME, sizeof(m_szTableName));
    m_pstConfig = pstConfig;
    m_szBinBuf = szBinBuff;
    m_stMysqlHandler.ConnectDB(m_pstConfig->m_szDBAddr, m_pstConfig->m_wPort, m_pstConfig->m_szDBName,
        m_pstConfig->m_szUser, m_pstConfig->m_szPassword);
    return true;
}

int MineOreTable::GetData(uint64_t ullKeyList[], uint8_t bNum, OUT PKGMETA::DT_MINE_ORE_DATA* pstData, OUT uint8_t* pOutNum)
{
    *pOutNum = 0;
    if (bNum <= 0)
    {
        LOGWARN_r("GetData  OreData uNum is <%hhu>", bNum);
        return ERR_NONE;
    }

    //int iTableNum = GET_TABLE_NUM(ullKeyList[0]);

    //Select * From 表5 Where 名字 in('陈杰', '陈', '王洁', '王兵')
    char* pszSql = m_stMysqlHandler.GetSql();
    pszSql[0] = '\0';
    StrCat(pszSql, DB_SQL_STR_SIZE, " SELECT Uid, Version, OreBlob FROM %s WHERE Uid in ( ", m_szTableName);
    uint8_t i = 0;
    for (; i < bNum - 1; i++)
    {
        StrCat(pszSql, DB_SQL_STR_SIZE, "%lu ,", ullKeyList[i]);
    }
    StrCat(pszSql, DB_SQL_STR_SIZE, "%lu );", ullKeyList[i]);
    if (m_stMysqlHandler.Execute(pszSql) < 0)
    {
        LOGERR("excute sql failed: %s", m_stMysqlHandler.GetLastErrMsg());
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
    
    for (oRowIter.Begin(m_stMysqlHandler.GetResult()); !oRowIter.IsEnd(); oRowIter.Next())
    {
        i = 0;
        DT_MINE_ORE_DATA& rstData = pstData[*pOutNum];
        rstData.m_ullOreUid = (uint64_t)oRowIter.GetField(i++)->GetBigInteger();
        rstData.m_dwVersion = (uint32_t)oRowIter.GetField(i++)->GetInteger();
        rstData.m_stOreBlob.m_iLen = oRowIter.GetField(i++)->GetBinData((char*)rstData.m_stOreBlob.m_szData, sizeof(rstData.m_stOreBlob.m_szData));
        (*pOutNum)++;
    }

    if (iEffectRow != bNum)
    {
        LOGERR_r("GetData  OreData Count<%d> OK, but require Count<%d>", iEffectRow, (int)bNum);
    }
    return ERR_NONE;
}


int MineOreTable::UptData(PKGMETA::DT_MINE_ORE_DATA* pstData, uint8_t bNum)
{
    if (bNum <= 0)
    {
        LOGWARN_r("UptData  OreData uNum is <%hhu>", bNum);
        return ERR_NONE;
    }
    char* pszSql = m_stMysqlHandler.GetSql();
    pszSql[0] = '\0';
    //int iTableNum = GET_TABLE_NUM(pstData[0].m_ullOreUid);
    //replace into test_tbl(id, dr) values(1, '2'), (2, '3'), ...(x, 'y');


    StrCat(pszSql, DB_SQL_STR_SIZE, " REPLACE INTO %s (Uid, Version, OreBlob) VALUES ", m_szTableName);
    
    for (int i = 0; i < bNum ; i++)
    {
        if (!m_stMysqlHandler.BinToStr((char* )pstData[i].m_stOreBlob.m_szData, pstData[i].m_stOreBlob.m_iLen, m_szBinBuf, CDBWorkThread::CONST_MINE_ORE_INFO_BINBUF_SIZE))
        {
            LOGERR_r("OreBlob convert to mysql bin failed, Uid<%lu> Version<%u>", pstData[i].m_ullOreUid, pstData[i].m_dwVersion);
            continue;
        }
        StrCat(pszSql, DB_SQL_STR_SIZE, "(%lu, %u, '%s')", pstData[i].m_ullOreUid, pstData[i].m_dwVersion, m_szBinBuf);
		if (i < bNum - 1)
		{
			StrCat(pszSql, DB_SQL_STR_SIZE, ", ");
		}
    }

    if (m_stMysqlHandler.Execute(pszSql) < 0)
    {
        LOGERR_r("excute sql failed: %s", m_stMysqlHandler.GetLastErrMsg());
        return ERR_DB_ERROR;
    }
    //LOGRUN_r("UptOreData Count<%u> ok!", (uint32_t)bNum);
    return ERR_NONE;
}

int MineOreTable::DelData(uint64_t ullKeyList [], uint8_t bNum)
{
    if (bNum <= 0)
    {
        LOGWARN_r("DelData OreData uNum is <%hhu>", bNum);
        return ERR_NONE;
    }
    //int iTableNum = GET_TABLE_NUM(ullKeyList[0]);
    char* pszSql = m_stMysqlHandler.GetSql();
    pszSql[0] = '\0';
    StrCat(pszSql, DB_SQL_STR_SIZE,  "DELETE FROM  %s  WHERE Uid in ( ", m_szTableName);
    
    for (int i = 0; i < bNum - 1; i++)
    {
        StrCat(pszSql, DB_SQL_STR_SIZE, "%lu, ", ullKeyList[i]);
    }
    StrCat(pszSql, DB_SQL_STR_SIZE, "%lu) ", ullKeyList[bNum - 1]);

    if (m_stMysqlHandler.Execute(pszSql) < 0)
    {
        LOGERR_r("Del MinePlayer Num<%d> error %s", (int)bNum, m_stMysqlHandler.GetLastErrMsg());
        return ERR_DB_ERROR;
    }
    if( m_stMysqlHandler.AffectedRows() != (int)bNum)
    {
        LOGERR_r("DelData OreData Num<%d> Ok, but require del Num<%d>", m_stMysqlHandler.AffectedRows(), (int)bNum);
        return ERR_NONE;
    }
    LOGRUN_r("DelMineOreData Count<%u> ok!", (uint32_t)bNum);
    return ERR_NONE;

}






