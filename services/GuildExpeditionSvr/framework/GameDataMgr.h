#pragma once

#include "tresload/TGameDataMgr.h"
#include "tresload/ov_res_keywords.h"
#include "tresload/ov_res_public.h"
#include "tresload/ov_res_server.h"
#include "utils/singleton.h"
#include "macros.h"


/*
    目前先启动时全部加载所有gamedata,后期内存优化时按需动态加载，卸载
*/

using namespace std;

typedef TGameDataMgr<RESBASIC>						ResBasicMgr_t;



class CGameDataMgr : public TSingleton<CGameDataMgr>
{
public:
    CGameDataMgr(){}
    ~CGameDataMgr(){}

    bool Init();

private:

    DECL_INIT_RES_MGR( m_oResBasicMgr,					    "gamedata/ResData/public/basic/ResBasic.bytes" );


private:
    DECL_GETTER_REF(ResBasicMgr_t, m_oResBasicMgr,	            ResBasicMgr );
    
};
