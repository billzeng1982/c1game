#pragma once
#include "MsgBase_c.h"
#include "ss_proto.h"

using namespace PKGMETA;

class CSsMailPlayerStatNtf : public IMsgBase_c
{
public:
    virtual int HandleServerMsg(SSPKG* pstSsPkg);
};

class CSsMailStatNtf : public IMsgBase_c
{
public:
    virtual int HandleServerMsg(SSPKG* pstSsPkg);
};

class CSsMailSyncReq : public IMsgBase_c
{
public:
    virtual int HandleServerMsg(SSPKG* pstSsPkg);
};

class CSsMailAddReq : public IMsgBase_c
{
public:
    virtual int HandleServerMsg(SSPKG* pstSsPkg);
};

class CSsMailAddByIdReq : public IMsgBase_c
{
public:
    virtual int HandleServerMsg(SSPKG* pstSsPkg);
};

class CSsMailPubTableGetDataRsp : public IMsgBase_c
{
public:
    virtual int HandleServerMsg(SSPKG* pstSsPkg);
};

class CSsMailPriTableGetDataRsp : public IMsgBase_c
{
public:
    virtual int HandleServerMsg(SSPKG* pstSsPkg);
};

