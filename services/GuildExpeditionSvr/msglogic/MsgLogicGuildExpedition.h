#pragma once
#include "MsgBase_c.h"
#include "ss_proto.h"

class CSsGuildExpeditionGetPlayerDataRsp : public IMsgBase_c
{
public:
	virtual int HandleServerMsg(PKGMETA::SSPKG* pstSsPkg);
};

class CSsGuildExpeditionGetGuildDataRsp : public IMsgBase_c
{
public:
	virtual int HandleServerMsg(PKGMETA::SSPKG* pstSsPkg);
};



class CSsGuildExpeditionGetAllInfoReq : public IMsgBase_c
{
public:
    virtual int HandleServerMsg(PKGMETA::SSPKG* pstSsPkg);
};

class CSsGuildExpeditionUploadGuildInfoNtf : public IMsgBase_c
{
public:
	virtual int HandleServerMsg(PKGMETA::SSPKG* pstSsPkg);
};

class CSsGuildExpeditionSetFightInfoNtf : public IMsgBase_c
{
public:
	virtual int HandleServerMsg(PKGMETA::SSPKG* pstSsPkg);
};


class CSsGuildExpeditionFightRequestReq : public IMsgBase_c
{
public:
	virtual int HandleServerMsg(PKGMETA::SSPKG* pstSsPkg);

};

class CSsGuildExpeditionFightResultReq : public IMsgBase_c
{
public:
	virtual int HandleServerMsg(PKGMETA::SSPKG* pstSsPkg);

};

class CSsGuildExpeditionMatchReq : public IMsgBase_c
{
public:
	virtual int HandleServerMsg(PKGMETA::SSPKG* pstSsPkg);

};

class CSsGuildExpeditionGuildStateNtf : public IMsgBase_c
{
public:
	virtual int HandleServerMsg(PKGMETA::SSPKG* pstSsPkg);

};

