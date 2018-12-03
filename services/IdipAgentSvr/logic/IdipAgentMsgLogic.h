#pragma once
#include "MsgBase.h"
#include "tdr/tdr.h"

#define STATUS_OK 200

class LogQueryRsp_SS: public IMsgBase
{
public:
	LogQueryRsp_SS() {}
	~LogQueryRsp_SS() {}
	virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL);
};

class GmRsp_SS: public IMsgBase
{
public:
	GmRsp_SS() {}
	~GmRsp_SS() {}
	virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL);
};
class GM_DATABASE_OP_SS : public IMsgBase
{
public:
	GM_DATABASE_OP_SS() {}
	~GM_DATABASE_OP_SS() {}
	virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL);
};
