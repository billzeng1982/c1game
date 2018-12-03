#pragma once

#include "MsgBase.h"
#include "common_proto.h"

//CS协议
class MessageSendReq_CS : public IMsgBase
{
public:
    virtual int HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL );
};

class MessageGetBoxReq_CS : public IMsgBase
{
public:
    virtual int HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL );
};

class MessageDelBoxReq_CS : public IMsgBase
{
public:
    virtual int HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL );
};

//SS协议
class MessageSendRsp_SS : public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class MessageGetBoxRsp_SS : public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class MessageDelBoxRsp_SS : public IMsgBase
{
public:
    virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL);
};

class MessageAutoSendNtf_SS : public IMsgBase
{
public:
    virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL);
};


