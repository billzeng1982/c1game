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

typedef TGameDataMgr<RESBASIC>		ResBasicMgr_t;
typedef TGameDataMgr<RESSCORE>		ResScoreMgr_t;
typedef TGameDataMgr<RESMATCH>      ResMatchMgr_t;
class CGameDataMgr : public TSingleton<CGameDataMgr>
{
public:
    CGameDataMgr(){}
    ~CGameDataMgr(){}

    bool Init();

private:
    DECL_INIT_RES_MGR( m_oResBasicMgr,	"gamedata/ResData/public/basic/ResBasic.bytes" );
    DECL_INIT_RES_MGR( m_oResScoreMgr,	"gamedata/ResData/public/match/ResScore.bytes" );
    DECL_INIT_RES_MGR( m_oResMatchMgr,  "gamedata/ResData/public/match/ResMatch.bytes" );

private:
    DECL_GETTER_REF( ResBasicMgr_t, m_oResBasicMgr,	ResBasicMgr );
    DECL_GETTER_REF( ResScoreMgr_t,	m_oResScoreMgr,	ResScoreMgr );
    DECL_GETTER_REF( ResMatchMgr_t, m_oResMatchMgr, ResMatchMgr );

};


