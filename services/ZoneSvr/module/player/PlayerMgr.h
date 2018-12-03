#pragma once

#include <map>
#include "define.h"
#include "singleton.h"
#include "mempool.h"
#include "iterator.h"
#include "functors.h"
#include "list_i.h"
#include "Player.h"
#include "../../ZoneSvr.h"

class PlayerMgr: public TSingleton<PlayerMgr> {
private:
	typedef hash_map_t< const char*, Player*, __gnu_cxx::hash<const char*>, eqstr > PlayerAccountNameMap_t;
	typedef hash_map_t< const char*, Player*, __gnu_cxx::hash<const char*>, eqstr > PlayerRoleNameMap_t;
	typedef std::map<uint32_t, Player*> PlayerSessionMap_t;
	typedef std::map<uint64_t, Player*> PlayerUinMap_t;

	const static uint16_t PLAYER_UPDATE_FREQ = 500; // 玩家更新平率, ms
	const static uint16_t PLATER_UPDATE_NUM_PER_SECOND = 10;

public:
	PlayerMgr();
	PlayerMgr(uint32_t size);

	virtual ~PlayerMgr();

	bool Init( int iMaxPlayer );
	void Destory();

	Player* GetPlayerByRoleName(const char* pszName );
	Player* GetPlayerByUin(uint64_t ulUin);
	Player* GetPlayerBySessionId(uint32_t dwSessionId);
	Player* GetPlayerByAccountName(const char* pszName );
	//通过Sdk获得的UserName和选择登录服务器Id获取玩家
	Player* GetPlayerBySdkUserNameAndSvrId(const char* pszName, uint32_t dwSvrId);
	CMemPool<Player>& GetPlayerPool();

	Player* GetOnePlayer();

	Player* NewPlayer();
	int DeletePlayer(Player* poPlayer);

	void AddToAccountNameHM(Player* poPlayer);
	void DelFromAccountNameHM(Player* poPlayer);

	void AddToRoleNameHM(Player* poPlayer);
	void DelFromRoleNameHM(Player* poPlayer);

	void AddToPlayerSessMap(Player* poPlayer);
	void DelFromPlayerSessMap(Player* poPlayer);

	void AddToPlayerUinMap(Player* poPlayer);
	void DelFromPlayerUinMap(Player* poPlayer);

	void Update(bool bIdle);

	int GetUsedNum() {return m_oPlayerPool.GetUsedNum();}
	uint32_t GetMaxSize() {return m_iPoolSize;}

	void LogoutAllPlayers();

private:
	static const uint32_t cDefaultPoolSize = 1000;

	uint32_t m_iPoolSize;
	CMemPool<Player> m_oPlayerPool;
	CMemPool<Player>::UsedIterator m_oUptIter; // update iterator;

protected:
	PlayerAccountNameMap_t m_oPlayerAccountNameHM;
	PlayerAccountNameMap_t::iterator m_oAccNameIter;

	PlayerRoleNameMap_t m_oPlayerRoleNameHM;
	PlayerRoleNameMap_t::iterator m_oRoleNameIter;

	PlayerSessionMap_t m_oPlayerSessionMap;
	PlayerSessionMap_t::iterator m_oPlayerSessIter;

	PlayerUinMap_t m_oPlayerUinMap;
	PlayerUinMap_t::iterator m_oPlayerUinIter;
};

