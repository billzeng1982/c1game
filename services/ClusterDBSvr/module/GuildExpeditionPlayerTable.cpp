
#include "LogMacros.h"
#include "comm_func.h"
#include "strutil.h"
#include "TableDefine.h"
#include "./GuildExpeditionGuildTable.h"
#include "../ClusterDBSvr.h"
#include "../thread/DBWorkThread.h"


using namespace PKGMETA;



GuildExpeditionPlayerTable::GuildExpeditionPlayerTable()
{
	m_szBinBuf = NULL;
}

GuildExpeditionPlayerTable::~GuildExpeditionPlayerTable()
{
	m_szBinBuf = NULL;
}

bool GuildExpeditionPlayerTable::Init(CMysqlHandler* poMysqlHandler, const char* pstTableName, CLUSTERDBSVRCFG* pstConfig, char* szBinBuff)
{
	if (NULL == pstConfig || szBinBuff == NULL || pstTableName == NULL)
	{
		return false;
	}
	StrCpy(m_szTableName, pstTableName, sizeof(m_szTableName));
	m_pstConfig = pstConfig;
	m_szBinBuf = szBinBuff;
	m_stMysqlHandler.ConnectDB(m_pstConfig->m_szDBAddr, m_pstConfig->m_wPort, m_pstConfig->m_szDBName,
		m_pstConfig->m_szUser, m_pstConfig->m_szPassword);
	return true;
}

int GuildExpeditionPlayerTable::GetData(uint64_t ullKeyList[], int8_t bNum, OUT PKGMETA::DT_GUILD_EXPEDITION_PLAYER_DATA* pstData, OUT int8_t* pOutNum)
{
	*pOutNum = 0;
	if (bNum <= 0)
	{
		LOGWARN_r("GetData  PlayerData uNum is <%hhu>", bNum);
		return ERR_NONE;
	}


	//Select * From 表5 Where 名字 in('陈杰', '陈', '王洁', '王兵')
	char* pszSql = m_stMysqlHandler.GetSql();
	pszSql[0] = '\0';
	StrCat(pszSql, DB_SQL_STR_SIZE, " SELECT Uin, Version, PlayerBlob FROM %s WHERE Uin in ( ", m_szTableName);
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
		DT_GUILD_EXPEDITION_PLAYER_DATA& rstData = pstData[*pOutNum];
		rstData.m_ullUin = (uint64_t)oRowIter.GetField(i++)->GetBigInteger();
		rstData.m_dwVersion = (uint32_t)oRowIter.GetField(i++)->GetInteger();
		rstData.m_stPlayerBlob.m_iLen = oRowIter.GetField(i++)->GetBinData((char*)rstData.m_stPlayerBlob.m_szData, sizeof(rstData.m_stPlayerBlob.m_szData));
		(*pOutNum)++;
	}

	if (iEffectRow != bNum)
	{
		LOGERR_r("GetData  PlayerData Count<%d> OK, but require Count<%d>", iEffectRow, (int)bNum);
	}
	return ERR_NONE;
}


int GuildExpeditionPlayerTable::UptData(PKGMETA::DT_GUILD_EXPEDITION_PLAYER_DATA* pstData, int8_t bNum)
{
	if (bNum <= 0)
	{
		LOGWARN_r("UptData  PlayerData uNum is <%hhu>", bNum);
		return ERR_NONE;
	}
	char* pszSql = m_stMysqlHandler.GetSql();
	pszSql[0] = '\0';
	//int iTableNum = GET_TABLE_NUM(pstData[0].m_ullOreUid);
	//replace into test_tbl(id, dr) values(1, '2'), (2, '3'), ...(x, 'y');


	StrCat(pszSql, DB_SQL_STR_SIZE, " REPLACE INTO %s (Uin, Version, PlayerBlob) VALUES ", m_szTableName);

	for (int i = 0; i < bNum; i++)
	{
		if (!m_stMysqlHandler.BinToStr((char*)pstData[i].m_stPlayerBlob.m_szData, pstData[i].m_stPlayerBlob.m_iLen, m_szBinBuf, CDBWorkThread::CONST_BINBUF_SIZE))
		{
			LOGERR_r("PlayerBlob convert to mysql bin failed, Uid<%lu> Version<%u>", pstData[i].m_ullUin, pstData[i].m_dwVersion);
			continue;
		}
		StrCat(pszSql, DB_SQL_STR_SIZE, "(%lu, %u, '%s')", pstData[i].m_ullUin, pstData[i].m_dwVersion, m_szBinBuf);
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








