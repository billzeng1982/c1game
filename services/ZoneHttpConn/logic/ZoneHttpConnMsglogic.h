#pragma once

#ifndef ZONE_HTTP_CONN_MSGLOGIC_H_
#define ZONE_HTTP_CONN_MSGLOGIC_H_

#include "msglayer/MsgBase.h"
#include "protocol/PKGMETA/ss_proto.h"

class GetPlayerWebTaskInfoRsp : public IMsgBase
{
public:
    GetPlayerWebTaskInfoRsp() {}
    ~GetPlayerWebTaskInfoRsp() {}

    int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL);
};

class GetWebTaskRwdRsp : public IMsgBase
{
public:
    GetWebTaskRwdRsp() {}
    ~GetWebTaskRwdRsp() {}

    int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL);
};

#endif
