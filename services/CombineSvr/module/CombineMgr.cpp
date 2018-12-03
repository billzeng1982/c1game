
#include "CombineMgr.h"
#include "LogMacros.h"
#include "BaseTable.h"
#include "TableDefine.h"
#include "RoleTable.h"
#include "CommonTable.h"

extern bool g_bExit;
/// 必须成对出现
/// 如果有新的不需要重名的表直接加这里
static const char * g_CombineTableList[] = {
	ROLE_TABLE_NAME,					TABLE_COLUMN_ROLE,
	ACCOUNT_TABLE_NAME,					TABLE_COLUMN_ACCOUNT,
	ASYNCPVP_PLAYER_TABLE_NAME,			TABLE_COLUMN_ASYNCPVP_PLAYER,
	ASYNCPVP_TEAM_TABLE_NAME,			TABLE_COLUMN_ASYNCPVP_TEAM,
	//MAIL_PUB_TABLE_NAME,				TABLE_COLUMN_MAIL_PUB,
	MAIL_PRI_TABLE_NAME,				TABLE_COLUMN_MAIL_PRI,
	GUILD_PLAYER_TABLE_NAME,			TABLE_COLUMN_GUILDPLAYER,
	MESSAGE_TABLE_NAME,					TABLE_COLUMN_MESSAGE,
	//MINE_ORE_TABLE_NAME,				TABLE_COLUMN_MINE_ORE,
	//MINE_PLAYER_TABLE_NAME,			TABLE_COLUMN_MINE_PLAYER,
	CLONEBATTLE_TEAM_TABLE_NAME,		TABLE_COLUMN_CLONEBATTLE_TEAM,
	FRIEND_TABLE_NAME,					TABLE_COLUMN_FRIEND,
	GUILD_TABLE_NAME,					TABLE_COLUMN_GUILD,
};


bool CombineMgr::Init(COMBINESVRCFG* pstConfig)
{
	m_pstConfig = pstConfig;

	key_t iShmKey = 0;
	ThreadInitPara stThreadInitPara = { 0 };
	stThreadInitPara.m_iTableSvrId = m_pstConfig->m_beCombinedSidList[0];
	stThreadInitPara.m_pstCombineSvrCfg = m_pstConfig;
	int iThreadNum = 0;
	int iCommonTableIndex = 0;
	CMysqlWorkThread* WorkThread = NULL;
	while (iCommonTableIndex < (int)(sizeof(g_CombineTableList)/sizeof(g_CombineTableList[0])))
	{
		stThreadInitPara.m_pstTableName = g_CombineTableList[iCommonTableIndex++];
		stThreadInitPara.m_pstTableColumn = g_CombineTableList[iCommonTableIndex++];
		WorkThread = new CMysqlWorkThread;
		if (!WorkThread || !WorkThread->InitThread(iThreadNum++, 1024 * 1024, THREAD_QUEUE_DUPLEX, (void*)&stThreadInitPara, &iShmKey))
		{
			LOGERR_r("Init thread <%d> failed", iThreadNum);
			return false;
		}
		m_astMysqlWorkThreads.push_back(WorkThread);
		m_CombineState.push_back(0);
	}

	return true;
}

void CombineMgr::ChangeState(int iIndex, int iResult)
{
	if (iIndex <0 || iIndex >= (int)m_CombineState.size())
	{
		LOGERR_r("ChangeState error, index<%d>", iIndex);
	}
	m_CombineState[iIndex] = iResult;
}

void CombineMgr::Fini()
{

	for (int i = 0; i < (int)m_astMysqlWorkThreads.size(); i++)
	{
		m_astMysqlWorkThreads[i]->FiniThread();
	}
	m_astMysqlWorkThreads.clear();
}

void CombineMgr::Update()
{
	bool bIsFini = true;
	for (int i = 0; i< (int)m_CombineState.size(); i++)
	{
		if (m_CombineState[i] == 0)
		{
			bIsFini = false;
			break;
		}
	}
	if (bIsFini)
	{
		//框架退出
		g_bExit = true;
	}
}

