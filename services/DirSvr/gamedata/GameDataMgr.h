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

typedef TGameDataMgr<RESSERVER>              ResServerMgr_t;


class CGameDataMgr : public TSingleton<CGameDataMgr>
{
public:
    CGameDataMgr(){}
    ~CGameDataMgr(){}

    bool Init();

    bool Reload();

private:
    DECL_INIT_RES_MGR(m_oResServerMgr,                      "gamedata/ResData/server/server/ResServer.bytes");
private:
    DECL_GETTER_REF( ResServerMgr_t,                 m_oResServerMgr,                 ResServerMgr );
};
