#pragma once

#include "MsgBase.h"
#include "common_proto.h"

//CS协议
class GuildExpeditionGetAllInfoReq_CS : public IMsgBase
{
public:
	virtual int HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL);
};

class GuildExpeditionSetBattleArrayReq_CS : public IMsgBase
{
public:
	virtual int HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL);
};

class GuildExpeditionGetFightRequestReq_CS : public IMsgBase
{
public:
	virtual int HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL);
};

class GuildExpeditionGetFightResultReq_CS : public IMsgBase
{
public:
	virtual int HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL);
};

class GuildExpeditionMatchReq_CS : public IMsgBase
{
public:
	virtual int HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL);
};

//SS协议======================================================================

class GuildExpeditionGetAllInfoRsp_SS : public IMsgBase
{
public:
	virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL);
};

class GuildExpeditionGetFightRequestRsp_SS : public IMsgBase
{
public:
	virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL);
};

class GuildExpeditionGetFightResultRsp_SS : public IMsgBase
{
public:
	virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL);
};

class GuildExpeditionMatchRsp_SS : public IMsgBase
{
public:
	virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL);
};

