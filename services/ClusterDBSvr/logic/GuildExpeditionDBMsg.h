#pragma once
#include "MsgBase_r.h"
#include "ss_proto.h"

class GuildExpeditionGetPlayerDataReq_SS : public IMsgBase_r
{
public:
	virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL);

private:
	PKGMETA::SSPKG 	m_stSsPkg;
};

class GuildExpeditionGetGuildDataReq_SS : public IMsgBase_r
{
public:
	virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL);

private:
	PKGMETA::SSPKG 	m_stSsPkg;
};



class GuildExpeditionUptPlayerDataNtf_SS : public IMsgBase_r
{
public:
	virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL);

private:
	PKGMETA::SSPKG 	m_stSsPkg;
};


class GuildExpeditionUptGuildDataNtf_SS : public IMsgBase_r
{
public:
	virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL);

private:
	PKGMETA::SSPKG 	m_stSsPkg;
};


class GuildExpeditionDelGuildDataNtf_SS : public IMsgBase_r
{
public:
	virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL);

private:
	PKGMETA::SSPKG 	m_stSsPkg;
};

