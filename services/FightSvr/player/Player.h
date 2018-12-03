#pragma once

#include "object.h"
#include "define.h"
#include "ss_proto.h"

class Dungeon;

/*
	战场中的player
*/

class Player : public IObject
{
public:
	static Player* Get();
	static void Release( Player* pObj );

public:
	Player();
	virtual ~Player() {}
	virtual void Clear();
	virtual void Update(int iDeltaTime);

	Dungeon* getDungeon() { return m_poDungeon; }
	void setDungeon(Dungeon* poDungeon);
	
	bool IsOnline() { return m_bOnline; }
	void setOnline( bool bVal ) { m_bOnline = bVal; }

	void SetProtocolVersion(uint16_t wVersion) { m_wVersion = wVersion;}
	uint16_t GetProtocolVersion() { return m_wVersion; }
	
private:
	void _Construct();

public:
	uint64_t m_ullUin;
	char m_szName[PKGMETA::MAX_NAME_LENGTH];
	PKGMETA::CONNSESSION m_stConnSession;
		
private:
	Dungeon* m_poDungeon;
	uint16_t m_wVersion;
	bool m_bOnline;	
};


