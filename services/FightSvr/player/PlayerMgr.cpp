#include "PlayerMgr.h"
#include "LogMacros.h"

bool PlayerMgr::Init()
{
	m_mapName2Player.clear();
	m_mapSession2Player.clear();
	m_mapUin2Player.clear();
	return true;
}

Player* PlayerMgr::New()
{
	Player* poPlayer = Player::Get();

	LOGRUN("new player");
	return poPlayer;
}

void PlayerMgr::Delete(Player* poPlayer)
{
	if (poPlayer == NULL)
	{
		return;
	}

	this->DelIndex(poPlayer);

	LOGRUN("delete player");
	Player::Release(poPlayer);
}

Player* PlayerMgr::GetByUin(uint64_t ullUin)
{
    MapUin2Player_t::iterator it = m_mapUin2Player.find( ullUin );
    if( it != m_mapUin2Player.end() )
    {
        return it->second;
    }
    return NULL;
}

Player* PlayerMgr::GetBySessionId(uint32_t dwSessionId)
{
    MapSession2Player_t::iterator it = m_mapSession2Player.find( dwSessionId );
    if( it != m_mapSession2Player.end() )
    {
        return it->second;
    }
    return NULL;
}

Player* PlayerMgr::GetByName( char* pszName )
{
    MapName2Player_t::iterator it = m_mapName2Player.find(pszName);
    if( it != m_mapName2Player.end())
    {
        return it->second;
    }
    return NULL;
}

void PlayerMgr::AddToNameMap( Player* poPlayer )
{
    if( NULL == poPlayer )
    {
        assert( false );
        return;
    }

    m_mapName2Player.insert( MapName2Player_t::value_type( poPlayer->m_szName, poPlayer ));
	return;
}

void PlayerMgr::DelFromNameMap( Player* poPlayer )
{
    if( NULL == poPlayer )
    {
        assert( false );
        return;
    }
    
    MapName2Player_t::iterator it = m_mapName2Player.find( poPlayer->m_szName );
    if( it != m_mapName2Player.end() )
    {
        m_mapName2Player.erase( it );
    }
    return;
}
		
void PlayerMgr::AddToSessionMap( Player* poPlayer )
{
    if( NULL == poPlayer || 0 == poPlayer->m_stConnSession.m_dwSessionId )
    {
        assert( false );
        return;
    }

    m_mapSession2Player.insert( MapSession2Player_t::value_type( poPlayer->m_stConnSession.m_dwSessionId, poPlayer ) );
}

void PlayerMgr::DelFromSessionMap( Player* poPlayer )
{
    if( NULL == poPlayer )
    {
        assert( false );
        return;
    }

    if( 0 == poPlayer->m_stConnSession.m_dwSessionId )
    {
        return;
    }

    MapSession2Player_t::iterator it = m_mapSession2Player.find( poPlayer->m_stConnSession.m_dwSessionId );
    if( it != m_mapSession2Player.end() )
    {
        m_mapSession2Player.erase( it );
    }
    return;
}

void PlayerMgr::AddToUinMap( Player* poPlayer )
{
    if( NULL == poPlayer || 0 == poPlayer->m_ullUin )
    {
        assert( false );
        return;
    }

    m_mapUin2Player.insert( MapUin2Player_t::value_type( poPlayer->m_ullUin, poPlayer ) );
}

void PlayerMgr::DelFromUinMap( Player* poPlayer )
{
    if( NULL == poPlayer || 0 == poPlayer->m_ullUin )
    {
        return;
    }

    MapUin2Player_t::iterator it = m_mapUin2Player.find( poPlayer->m_ullUin );
    if( it != m_mapUin2Player.end() )
    {
        m_mapUin2Player.erase(it);
    }
    return;
}

void PlayerMgr::AddIndex(Player* poPlayer)
{
	if (poPlayer == NULL)
	{
		return;
	}

	this->AddToSessionMap(poPlayer);
	this->AddToUinMap(poPlayer);
	this->AddToNameMap(poPlayer);
}

void PlayerMgr::DelIndex(Player* poPlayer)
{
	if (poPlayer == NULL)
	{
		return;
	}

	this->DelFromSessionMap(poPlayer);
	this->DelFromUinMap(poPlayer);
	this->DelFromNameMap(poPlayer);
}
