#pragma once
#include "define.h"
#include "common_proto.h"
#include "cs_proto.h"
#include "singleton.h"
#include "player/PlayerData.h"
#include "../gamedata/GameDataMgr.h"
class PropSynthetic : public TSingleton<PropSynthetic>
{
public:
    PropSynthetic() {}
    virtual ~PropSynthetic(){}

public:
    int Synthetic(PlayerData* pstData,  CS_PKG_PROP_SYNTHETIC_REQ& rstReq,  SC_PKG_PROP_SYNTHETIC_RSP& rstRsp); 
    
    int SyntheticSomeOne(PlayerData* pstData, RESSYNTHETIC* poResSynThetic, CS_PKG_PROP_SYNTHETIC_REQ& rstReq,  SC_PKG_PROP_SYNTHETIC_RSP& rstRsp);   //����Ʒ�ϳ�һ
    int  SyntheticRandOne(PlayerData* pstData, RESSYNTHETIC* poResSynThetic, CS_PKG_PROP_SYNTHETIC_REQ& rstReq,  SC_PKG_PROP_SYNTHETIC_RSP& rstRsp);  //ͬƷ��X��һ
};


