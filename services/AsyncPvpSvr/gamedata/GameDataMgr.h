#pragma once

#include "TGameDataMgr.h"
#include "ov_res_keywords.h"
#include "ov_res_public.h"
#include "ov_res_server.h"
#include "singleton.h"
#include "macros.h"


/*
    目前先启动时全部加载所有gamedata,后期内存优化时按需动态加载，卸载
*/

using namespace std;

typedef TGameDataMgr<RESASYNCPVPFAKEPLAYER>		 ResFakePlayerMgr_t;
typedef TGameDataMgr<RESASYNCPVPRANKREWARD>		 ResRankRewardMgr_t;
typedef TGameDataMgr<RESPRIMAIL>                 ResPriMailMgr_t;
typedef TGameDataMgr<RESBASIC>				     ResBasicMgr_t;


class CGameDataMgr : public TSingleton<CGameDataMgr>
{
public:
    CGameDataMgr(){}
    ~CGameDataMgr(){}

    bool Init();

private:
    DECL_INIT_RES_MGR( m_oResFakePlayerMgr,	   "gamedata/ResData/server/asyncpvp/ResAsyncPVPFakePlayer.bytes" );
    DECL_INIT_RES_MGR( m_oResRankRewardMgr,    "gamedata/ResData/public/asyncpvp/ResAsyncPVPRankReward.bytes" );
    DECL_INIT_RES_MGR( m_oResPriMailMgr,	   "gamedata/ResData/server/mail/ResPriMail.bytes" );
    DECL_INIT_RES_MGR( m_oResBasicMgr,         "gamedata/ResData/public/basic/ResBasic.bytes" );

private:
    DECL_GETTER_REF( ResFakePlayerMgr_t,	 m_oResFakePlayerMgr,	  ResFakePlayerMgr);
    DECL_GETTER_REF( ResRankRewardMgr_t,     m_oResRankRewardMgr,     ResRankRewardMgr);
    DECL_GETTER_REF( ResPriMailMgr_t,	     m_oResPriMailMgr,	      ResPriMailMgr );
    DECL_GETTER_REF( ResBasicMgr_t,				    m_oResBasicMgr,					ResBasicMgr );
};
