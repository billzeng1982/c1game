#pragma once
#include "MsgBase_r.h"
#include "Gm.h"

// 在多线程里处理消息

class CSsRoleGmUpdateInfoReq : public IMsgBase_r
{
public:
    CSsRoleGmUpdateInfoReq(){}
    virtual ~CSsRoleGmUpdateInfoReq(){}
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
private:
    GmMgr m_oGmMgr;
};

