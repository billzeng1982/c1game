#pragma once
#include "MsgBase.h"
#include "tdr/tdr.h"
#include "idip_proto.h"

class IdipReq_SS: public IMsgBase
{
public:
    virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );

private:
    void _HandleGetPlayerLogMsg(IdipPkg& rstIdipPkg, uint64_t ullKey);

private:
    IdipPkg m_stIdipPkg;
};





