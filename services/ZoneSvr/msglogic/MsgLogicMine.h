#pragma once

#include "MsgBase.h"
#include "common_proto.h"

//CS协议
class MineGetInfoReq_CS : public IMsgBase
{
public:
    virtual int HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL);
};

class MineExploreReq_CS : public IMsgBase
{
public:
    virtual int HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL);
};

class MineDealOreReq_CS : public IMsgBase
{
public:
    virtual int HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL);
};


class MineGetAwardReq_CS : public IMsgBase
{
public:
    virtual int HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL);
};

class MineFightSettleReq_CS : public IMsgBase
{
public:
    virtual int HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL);
};

class MineBuyCountReq_CS : public IMsgBase
{
public:
    virtual int HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL);
};


class MineFightResultReq_CS : public IMsgBase
{
public:
	virtual int HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL);
};

class MineGetRevengerInfoReq_CS : public IMsgBase
{
public:
	virtual int HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL);
};

//SS协议
class MineGetInfoRsp_SS : public IMsgBase
{
public:
    virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL);
};

class MineExploreRsp_SS : public IMsgBase
{
public:
    virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL);
};

class MineDealOreRsp_SS : public IMsgBase
{
public:
    virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL);
};

class MineGetAwardRsp_SS : public IMsgBase
{
public:
    virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL);
};

class MineInfoNtf_SS : public IMsgBase
{
public:
    virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL);
};

class MineFightResultRsp_SS : public IMsgBase
{
public:
	virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL);
};

class MineGetRevengerInfoRsp_SS : public IMsgBase
{
public:
	virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL);
};

class MineSeasonSettleNtf_SS : public IMsgBase
{
public:
	virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL);
};

