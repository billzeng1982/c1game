#pragma once
#include "MsgBase_r.h"
#include "tdr/tdr.h"
#include "ss_proto.h"

class MineGetOreDataReq_SS : public IMsgBase_r
{
public:
    virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL);

private:
    PKGMETA::SSPKG 	m_stSsPkg;
};

class MineUptOreDataNtf_SS : public IMsgBase_r
{
public:
    virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL);

private:
    PKGMETA::SSPKG 	m_stSsPkg;
};

class MineDelOreDataNtf_SS : public IMsgBase_r
{
public:
    virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL);

private:
    PKGMETA::SSPKG 	m_stSsPkg;
};


class MineGetPlayerDataReq_SS : public IMsgBase_r
{
public:
    virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL);

private:
    PKGMETA::SSPKG 	m_stSsPkg;
};

class MineUptPlayerDataNtf_SS : public IMsgBase_r
{
public:
    virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL);

private:
    PKGMETA::SSPKG 	m_stSsPkg;
};
