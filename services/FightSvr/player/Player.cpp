#include "Player.h"
#include "PlayerMgr.h"
#include "../framework/GameObjectPool.h"
#include "../dungeon/Dungeon.h"

Player::Player()
{
    this->_Construct();
}

void Player::_Construct()
{
    bzero( &m_stConnSession, sizeof(m_stConnSession) );

	m_poDungeon = NULL;
    m_wVersion = 1;
    m_bOnline = false;
}

void Player::Clear()
{
	if (m_poDungeon != NULL)
	{
		m_poDungeon->SetFightPlayerByUin(m_ullUin, NULL);
	}

    this->_Construct();

    IObject::Clear();
}

Player* Player::Get()
{
    return GET_GAMEOBJECT( Player, GAMEOBJ_PLAYER );
}

void Player::Release( Player* pObj )
{
    RELEASE_GAMEOBJECT( pObj );
}

void Player::Update(int iDeltaTime)
{
}

void Player::setDungeon(Dungeon* poDungeon)
{
	// 如果有老副本，则清除老副本中相应信息
	if (m_poDungeon != NULL)
	{
		m_poDungeon->SetFightPlayerByUin(m_ullUin, NULL);
	}

	// 置为新副本，并更新新副本相应信息
	m_poDungeon = poDungeon;
	if (m_poDungeon != NULL)
	{
		m_poDungeon->SetFightPlayerByUin(m_ullUin, this);
	}
}
