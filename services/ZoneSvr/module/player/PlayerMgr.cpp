#include "LogMacros.h"
#include "hash_func.h"
#include "og_comm.h"
#include "common_proto.h"
#include "PlayerLogic.h"
#include "PlayerMgr.h"
#include "../../ZoneSvr.h"
#include "PlayerDataDynPool.h"
#include "CpuSampleStats.h"

PlayerMgr::PlayerMgr()
{
	m_iPoolSize = cDefaultPoolSize;
}

PlayerMgr::PlayerMgr(uint32_t size) : m_iPoolSize(size) {}

PlayerMgr::~PlayerMgr() {}

bool PlayerMgr::Init(int iMaxPlayer)
{
	if (m_oPlayerPool.CreatePool(iMaxPlayer == 0 ? m_iPoolSize : iMaxPlayer) < 0)
    {
		LOGERR("Create oBuffPlayer pool[num=%d] failed.", m_iPoolSize);
		return false;
	}

	if (0 != iMaxPlayer)
    {
		m_iPoolSize = iMaxPlayer;
	}

    m_oPlayerPool.RegisterSlicedIter(&m_oUptIter);

    if( !PlayerDataDynPool::Instance().Init() )
    {
        LOGERR("Init player data dyn pool failed!");
        return false;
    }

	return true;
}

void PlayerMgr::Destory()
{
	m_oPlayerAccountNameHM.clear();
	m_oPlayerRoleNameHM.clear();
	m_oPlayerSessionMap.clear();
	m_oPlayerUinMap.clear();
	m_oPlayerPool.DestroyPool();
	return;
}

Player* PlayerMgr::GetPlayerByUin(uint64_t ulUin)
{
	m_oPlayerUinIter = m_oPlayerUinMap.find(ulUin);
	if (m_oPlayerUinIter != m_oPlayerUinMap.end())
    {
		return m_oPlayerUinIter->second;
	}
	return NULL;
}

Player* PlayerMgr::GetPlayerBySessionId(uint32_t dwSessionId)
{
	m_oPlayerSessIter = m_oPlayerSessionMap.find(dwSessionId);
	if (m_oPlayerSessIter != m_oPlayerSessionMap.end())
    {
		return m_oPlayerSessIter->second;
	}

	return NULL;
}

Player* PlayerMgr::GetPlayerByRoleName(const char* pszName)
{
	m_oRoleNameIter = m_oPlayerRoleNameHM.find(pszName);
	if (m_oRoleNameIter != m_oPlayerRoleNameHM.end())
    {
		return m_oRoleNameIter->second;
	}
	return NULL;
}

Player* PlayerMgr::GetPlayerByAccountName(const char* pszName)
{
	m_oAccNameIter = m_oPlayerAccountNameHM.find(pszName);
	if (m_oAccNameIter != m_oPlayerAccountNameHM.end())
    {
		return m_oAccNameIter->second;
	}
	return NULL;
}


Player* PlayerMgr::GetPlayerBySdkUserNameAndSvrId(const char* pszName, uint32_t dwSvrId)
{
	char Key[MAX_NAME_LENGTH] = { 0 };
	snprintf(Key, MAX_NAME_LENGTH, "%s_%u", pszName, dwSvrId);
	Key[MAX_NAME_LENGTH - 1] = '\0';
	m_oAccNameIter = m_oPlayerAccountNameHM.find(Key);
	if (m_oAccNameIter != m_oPlayerAccountNameHM.end())
	{
		return m_oAccNameIter->second;
	}
	return NULL;
}

CMemPool<Player>& PlayerMgr::GetPlayerPool()
{
	return m_oPlayerPool;
}

Player* PlayerMgr::NewPlayer()
{
	Player* poPlayer = NULL;
	poPlayer = m_oPlayerPool.NewData();
	return poPlayer;
}

int PlayerMgr::DeletePlayer(Player* poPlayer)
{
	if (!poPlayer)
    {
        LOGERR("poPlayer is null");
		return -1;
	}

	// delete from all dict
	this->DelFromAccountNameHM(poPlayer);
	this->DelFromRoleNameHM(poPlayer);
	this->DelFromPlayerSessMap(poPlayer);
	this->DelFromPlayerUinMap(poPlayer);

	poPlayer->Reset();

	return m_oPlayerPool.DeleteData(poPlayer);
}

void PlayerMgr::AddToAccountNameHM(Player* poPlayer)
{
	if (NULL == poPlayer)
    {
		LOGRUN("poPlayer is null");
		return;
	}

    char* szName = poPlayer->GetAccountName();
    if ('\0' == szName[0] )
    {
        LOGRUN("poPlayer account name is null");
        return;
    }

	m_oPlayerAccountNameHM.insert(PlayerAccountNameMap_t::value_type(poPlayer->GetAccountName(), poPlayer));

	return;
}

void PlayerMgr::DelFromAccountNameHM(Player* poPlayer)
{
	if (NULL == poPlayer)
    {
		LOGRUN("poPlayer is null");
		return;
	}

    char* szName = poPlayer->GetAccountName();
    if ('\0' == szName[0] )
    {
        LOGRUN("poPlayer account name is null");
        return;
    }

    LOGRUN("Del Player Account(%s) from AccMap", poPlayer->GetAccountName());

	m_oAccNameIter = m_oPlayerAccountNameHM.find(poPlayer->GetAccountName());
	if (m_oAccNameIter != m_oPlayerAccountNameHM.end())
    {
		m_oPlayerAccountNameHM.erase(m_oAccNameIter);
	}
	return;
}

void PlayerMgr::AddToRoleNameHM(Player* poPlayer)
{
	if (NULL == poPlayer)
    {
		LOGRUN("poPlayer is null");
		return;
	}

	m_oPlayerRoleNameHM.insert(PlayerRoleNameMap_t::value_type(poPlayer->GetRoleName(), poPlayer));
	return;
}

void PlayerMgr::DelFromRoleNameHM(Player* poPlayer)
{
	if (NULL == poPlayer)
    {
		LOGRUN("poPlayer is null");
		return;
	}

	m_oRoleNameIter = m_oPlayerRoleNameHM.find(poPlayer->GetRoleName());
	if (m_oRoleNameIter != m_oPlayerRoleNameHM.end())
    {
		m_oPlayerRoleNameHM.erase(m_oRoleNameIter);
	}
	return;
}

void PlayerMgr::AddToPlayerSessMap(Player* poPlayer)
{
	if (NULL == poPlayer || 0 == poPlayer->GetConnSessionID())
    {
		LOGRUN("poPlayer is null");
		return;
	}

	m_oPlayerSessionMap.insert(PlayerSessionMap_t::value_type(poPlayer->GetConnSessionID(), poPlayer));
}

void PlayerMgr::DelFromPlayerSessMap(Player* poPlayer)
{
	if (NULL == poPlayer || (0 == poPlayer->GetConnSessionID()))
    {
		LOGRUN("poPlayer is null");
		return;
	}

	m_oPlayerSessIter = m_oPlayerSessionMap.find(poPlayer->GetConnSessionID());
	if (m_oPlayerSessIter != m_oPlayerSessionMap.end())
    {
		m_oPlayerSessionMap.erase(m_oPlayerSessIter);
	}
	return;
}

void PlayerMgr::AddToPlayerUinMap(Player* poPlayer)
{
	if (NULL == poPlayer || 0 == poPlayer->GetUin())
    {
		LOGRUN("poPlayer is null");
		return;
	}

	m_oPlayerUinMap.insert(PlayerUinMap_t::value_type(poPlayer->GetUin(), poPlayer));
}

void PlayerMgr::DelFromPlayerUinMap(Player* poPlayer)
{
	if (NULL == poPlayer || 0 == poPlayer->GetUin())
    {
        LOGRUN("poPlayer is null");
		return;
	}

    LOGRUN("Del Player Uin(%lu) from UinMap", poPlayer->GetUin());

	m_oPlayerUinIter = m_oPlayerUinMap.find(poPlayer->GetUin());
	if (m_oPlayerUinIter != m_oPlayerUinMap.end())
    {
		m_oPlayerUinMap.erase(m_oPlayerUinIter);
	}
	return;
}

void PlayerMgr::LogoutAllPlayers()
{
	m_oUptIter.Begin();

	PlayerLogic& roPlayerLogic = PlayerLogic::Instance();
	Player* poPlayer = NULL;
	for ( int i = 1; !m_oUptIter.IsEnd(); i++, m_oUptIter.Next() )
    {
		poPlayer = m_oUptIter.CurrItem();
		if (NULL == poPlayer)
        {
			LOGERR("poPlayer is null");
			break;
		}

		//退出时确保每个玩家的数据写回DB中
		poPlayer->UptRoleDataToDB();

		roPlayerLogic.PlayerLogout(poPlayer, PLAYER_LOGOUT_BY_LOGIC, NULL);

		if (i % PLATER_UPDATE_NUM_PER_SECOND == 0)
        {
			MsSleep(10);
		}
	}

	return;
}

void PlayerMgr::Update(bool bIdle)
{
	static struct timeval tvUptStartTime = { 0 };
	struct timeval tvCurTime = { 0 };

	if (m_oPlayerPool.GetUsedNum() == 0)
    {
		return;
	}

	tvCurTime = *(CGameTime::Instance().GetCurrTime());

	if (MsPass(&tvCurTime, &tvUptStartTime) < PLAYER_UPDATE_FREQ)
    {
		return;
	}

	int iCheckNum = bIdle ? 50 : 10;
	if (m_oUptIter.IsEnd())
    {
		m_oUptIter.Begin();
	}

	Player* poPlayer = NULL;
	int i = 0;
	for (; i < iCheckNum && !m_oUptIter.IsEnd(); i++, m_oUptIter.Next())
    {
		poPlayer = m_oUptIter.CurrItem();
		if (NULL == poPlayer)
        {
			LOGERR("poPlayer is null");
			break;
		}

        CpuSampleStats::Instance().BeginSample("poPlayer->OnUpdate");

		// update player
		poPlayer->OnUpdate();

        CpuSampleStats::Instance().EndSample();


	}

	if ( m_oUptIter.IsEnd() )
    {
		tvUptStartTime = tvCurTime;
	}

}

Player* PlayerMgr::GetOnePlayer()
{
	m_oPlayerUinIter = m_oPlayerUinMap.begin();

	if (m_oPlayerUinIter != m_oPlayerUinMap.end())
	{
		return m_oPlayerUinIter->second;
	}

	return NULL;
}
