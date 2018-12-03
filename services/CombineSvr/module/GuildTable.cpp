

#include "LogMacros.h"
#include "GuildTable.h"
#include "comm_func.h"
#include "TableDefine.h"


GuildTable::GuildTable()
{
	m_szMaxBinBuf = NULL;

}
GuildTable::~GuildTable()
{
	SAFE_DEL_ARRAY(m_szMaxBinBuf);

}
bool GuildTable::Init(CMysqlHandler* poMysqlHandle, int iTableSvrId, const char* pstTableName, const char* pstTableColumn)
{
	if (!BaseTable::Init(poMysqlHandle, iTableSvrId, pstTableName, pstTableColumn))
	{
		return false;
	}
	m_szMaxBinBuf = new char[MAX_BINBUF_SIZE];
	if (!m_szMaxBinBuf) return false;


	return true;
}

int GuildTable::Work()
{
	if (0 == Combine())
	{
		LOGRUN_r("[OK] Table %s_%d combine  OK!", m_szTableName, m_iTableSvrId);
		return 1;
	}
	else
	{
		LOGRUN_r("[ER] Table %s_%d combine  ERR!", m_szTableName, m_iTableSvrId);
		return -1;
	}
}

int GuildTable::Combine()
{
	if (ERR_NONE != GetLocalAllName("Name", 100))
	{
		return ERR_SYS;
	}
	int iIdStart = 0;
	int iDelta = 3;
	while (1)
	{
		m_poMysqlHandler->FormatSql("SELECT %s FROM `%s_%d` limit %d, %d ",
			m_szTableColumn, m_szTableName, m_iTableSvrId, iIdStart, iDelta);
		if (m_poMysqlHandler->Execute() < 0)
		{
			LOGERR_r("Table<%s> excute sql failed: %s", m_szTableName, m_poMysqlHandler->GetLastErrMsg());
			return ERR_DB_ERROR;
		}
		iIdStart += iDelta;
		int iGetSelectNum = m_poMysqlHandler->StoreResult();
		if (iGetSelectNum == 0)
		{
			break;
		}

		CMysqlRowIter& oRowIter = m_poMysqlHandler->GetRowIter();

		char* pszSql = m_poMysqlHandler->GetSql();
		pszSql[0] = '\0';
		//这里 insert的顺序 要与_ConvertWholeRoleToSql中的 转换顺序一致
		StrCat(pszSql, DB_SQL_STR_SIZE, " INSERT INTO `%s` ( %s )  VALUES ", m_szTableName, m_szTableColumn);
		for (oRowIter.Begin(m_poMysqlHandler->GetResult()); !oRowIter.IsEnd(); )
		{
			int i = 0;
			StrCat(pszSql, DB_SQL_STR_SIZE,
				"('%s',"	//Name
				"%lu,"		//GuildId
				"%u,"		//Version
				"%u"		//DelFlag
				" ",
				GetNewName(oRowIter.GetField(i)->GetString(), (uint64_t)oRowIter.GetField(i + 1)->GetBigInteger()),
				(uint64_t)oRowIter.GetField(i + 1)->GetBigInteger(),
				(uint32_t)oRowIter.GetField(i + 2)->GetInteger(),
				(uint32_t)oRowIter.GetField(i + 3)->GetInteger()
			);
			if (0 != _ConvertWholeBinToSql(GUILD_BLOB_COUNT, oRowIter, i + 4, pszSql, DB_SQL_STR_SIZE))
			{
				return ERR_SYS;
			}
			oRowIter.Next();
			if (oRowIter.IsEnd())
			{
				StrCat(pszSql, DB_SQL_STR_SIZE, ") ");
			}
			else
			{
				StrCat(pszSql, DB_SQL_STR_SIZE, "), ");
			}
		}
		MsSleep(3);
		if (m_poMysqlHandler->Execute(pszSql) < 0)
		{
			LOGERR_r("Table<%s> excute sql failed: %s", m_szTableName, m_poMysqlHandler->GetLastErrMsg());
			return ERR_DB_ERROR;
		}
	}
	//LOGRUN_r("Combine %s_%d OK!", m_szTableName, m_iTableSvrId);
	return ERR_NONE;
}

