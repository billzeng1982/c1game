#pragma once

#include "MsgBase.h"
#include "../module/player/Player.h"
#include "ss_proto.h"
#include "cs_proto.h"

using namespace PKGMETA;

class AccountLoginReq_CS: public IMsgBase
{
public:
    virtual int HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL);
    bool _CheckNameVaild(char* szName);
    int _HandleAccReconn(const PKGMETA::CONNSESSION* pstSession, Player* poPlayer, PKGMETA::CSPKG& rstCsPkg);
    int _HandleConnect(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg);
    int _HandleSDKLogin(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg);
};

class ReconnectReq_CS: public IMsgBase
{
public:
    virtual int HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL);

private:
    int _CheckSeqNo(uint32_t dwSeq, Player* poPlayer, uint32_t* pdwMinSeq);
    int _SendMissingPkg(uint32_t dwSeq, Player* poPlayer,  uint32_t dwMinSeq);

private:
    char* m_szBuffer;
    SCPkgHead m_stPkgHead;
};

class AccountLoginRsp_SS: public IMsgBase
{
public:
    virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL);
};

class SDKAccountLoginRsp_SS : public IMsgBase
{
public:
    virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL);
    int _HandleReconn(Player* poOldPlayer, Player* poNewPlayer);
    int _HandleSdkLogin(PKGMETA::SSPKG& rstSsPkg, Player* poPlayer);
};

class RoleCreateRsp_SS: public IMsgBase
{
public:
    virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL);
};

class RoleLoginReq_CS: public IMsgBase
{
public:
    virtual int HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL);
};

class RoleLoginRsp_SS: public IMsgBase
{
public:
    virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL);
};

class RoleUpdateRsp_SS: public IMsgBase
{
public:
    RoleUpdateRsp_SS(){}
    virtual ~RoleUpdateRsp_SS(){}
};

class AccountLogoutReq_CS: public IMsgBase
{
public:
    virtual int HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL);
};

class MajestyRegReq_CS: public IMsgBase
{
public:
    virtual int HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL);
};

class MajestyRegRsp_SS: public IMsgBase
{
public:
    virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL);
};



