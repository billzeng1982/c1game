#pragma once
#include "MsgBase_r.h"
#include "Gm.h"

// �ڶ��߳��ﴦ����Ϣ

class CSsRoleGmUpdateInfoReq : public IMsgBase_r
{
public:
    CSsRoleGmUpdateInfoReq(){}
    virtual ~CSsRoleGmUpdateInfoReq(){}
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
private:
    GmMgr m_oGmMgr;
};

