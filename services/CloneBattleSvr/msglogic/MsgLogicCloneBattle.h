#pragma once
#include "MsgBase_c.h"
#include "ss_proto.h"


class CSsCloneBattleGetInfoReq : public IMsgBase_c
{
public:
    virtual int HandleServerMsg(PKGMETA::SSPKG* pstSsPkg);
};

class CSsCloneBattleGetDataRsp : public IMsgBase_c
{
public:
    virtual int HandleServerMsg(PKGMETA::SSPKG* pstSsPkg);
};

class CSsCloneBattleJoinTeamReq : public IMsgBase_c
{
public:
    virtual int HandleServerMsg(PKGMETA::SSPKG* pstSsPkg);
};

class CSsCloneBattleFightReq : public IMsgBase_c
{
public:
    virtual int HandleServerMsg(PKGMETA::SSPKG* pstSsPkg);
};


class CSsCloneBattleRewardNtf : public IMsgBase_c
{
public:
    virtual int HandleServerMsg(PKGMETA::SSPKG* pstSsPkg);
};

class CSsCloneBattleQuitTeamReq : public IMsgBase_c
{
public:
    virtual int HandleServerMsg(PKGMETA::SSPKG* pstSsPkg);
};

class CSsCloneBattleSetTeamReq : public IMsgBase_c
{
public:
    virtual int HandleServerMsg(PKGMETA::SSPKG* pstSsPkg);
};

class CSsCloneBattleZoneSvrOnlineNtf : public IMsgBase_c
{
public:
    virtual int HandleServerMsg(PKGMETA::SSPKG* pstSsPkg);
};



