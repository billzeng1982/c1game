#pragma once

#include "TGameDataMgr.h"
#include "ov_res_server.h"
#include "singleton.h"
#include "macros.h"

typedef TGameDataMgr<RESACCOUNTIMPORTINTERNAL>			ResAccountMgr_t;

class CGameDataMgr : public TSingleton<CGameDataMgr>
{
public:
	CGameDataMgr(){}
	~CGameDataMgr(){}

	bool Init();
	bool Init( char* ResFile );

private:
	DECL_INIT_RES_MGR( m_oResAccountMgr, "/home/lzk/rgame/trunk_src/deploy/gamedata/server/account/ResAccountImportInternal.bin");
private:
	DECL_GETTER_REF( ResAccountMgr_t,		m_oResAccountMgr,		ResAccountMgr );
};

