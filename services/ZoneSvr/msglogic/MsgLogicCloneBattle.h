#pragma once

#include "MsgBase.h"

class CloneBattleGetInfoReq_CS : public IMsgBase
{
public:
    virtual int HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL);
};


class CloneBattleFightReq_CS : public IMsgBase
{
public:
    virtual int HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL);
};

class CloneBattleJoinTeamReq_CS : public IMsgBase
{
public:
    virtual int HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL);
};

class CloneBattleRewardReq_CS : public IMsgBase
{
public:
    virtual int HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL);
};

class CloneBattleQuitTeamReq_CS : public IMsgBase
{
public:
    virtual int HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL);
};

class CloneBattleSetTeamReq_CS : public IMsgBase
{
public:
    virtual int HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL);
};



class CloneBattleGetInfoRsp_SS : public IMsgBase
{
public:
    virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL);
};

class CloneBattleBroadcastNtf_SS : public IMsgBase
{
public:
    virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL);
};
class CloneBattleFightRsp_SS : public IMsgBase
{
public:
    virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL);
};
class CloneBattleJoinTeamRsp_SS : public IMsgBase
{
public:
    virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL);
};

class CloneBattleQuitTeamRsp_SS : public IMsgBase
{
public:
    virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL);
};

class CloneBattleSetTeamRsp_SS : public IMsgBase
{
public:
    virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL);
};

class CloneBattleUptSysInfoNtf_SS : public IMsgBase
{
public:
    virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL);
};
  
