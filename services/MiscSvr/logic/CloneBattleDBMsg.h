#pragma once
#include "MsgBase_r.h"
#include "tdr/tdr.h"
#include "ss_proto.h"

class CloneBattleDelDataNtf_SS : public IMsgBase_r
{
public:
    virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL);

private:
    PKGMETA::SSPKG 	m_stSsPkg;
};

class CloneBattleUptDataNtf_SS : public IMsgBase_r
{
public:
    virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL);

private:
    PKGMETA::SSPKG 	m_stSsPkg;
};

class CloneBattleGetDataReq_SS : public IMsgBase_r
{
public:
    virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL);

private:
    PKGMETA::SSPKG 	m_stSsPkg;
};

