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

typedef TGameDataMgr<RESSERIALNUM>	ResSerialMgr_t;

class CGameDataMgr : public TSingleton<CGameDataMgr>
{
public:
    CGameDataMgr(){}
    ~CGameDataMgr(){}

    bool Init();

private:
    DECL_INIT_RES_MGR(m_oResSerialMgr,	"gamedata/ResData/server/serialnum/ResSerialNum.bytes");

private:
    DECL_GETTER_REF(ResSerialMgr_t,	 m_oResSerialMgr,  ResSerialMgr);
};
