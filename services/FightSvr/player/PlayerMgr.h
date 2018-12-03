#pragma once

/*
	注意 manager 不管对象的分配释放
*/

#include <map>
#include "singleton.h"
#include "define.h"
#include "functors.h"
#include "Player.h"

class PlayerMgr : public TSingleton<PlayerMgr>
{
	typedef std::map<const char*, Player*> MapName2Player_t;
	typedef std::map<uint32_t, Player*> MapSession2Player_t;
	typedef std::map<uint64_t, Player*> MapUin2Player_t;

public:
	bool Init();

	Player* New();
	void Delete(Player* poPlayer);

	Player* GetByUin(uint64_t ulUin);
	Player* GetBySessionId(uint32_t dwSessionId);
	Player* GetByName(char* pszName);
	
	void AddToNameMap(Player* poPlayer);
	void DelFromNameMap(Player* poPlayer);
		
	void AddToSessionMap(Player* poPlayer);
	void DelFromSessionMap(Player* poPlayer);

	void AddToUinMap(Player* poPlayer);
	void DelFromUinMap(Player* poPlayer);

	void AddIndex(Player* poPlayer);
	void DelIndex(Player* poPlayer);

private:
	MapName2Player_t		m_mapName2Player;
	MapSession2Player_t 	m_mapSession2Player;
	MapUin2Player_t			m_mapUin2Player;
};

