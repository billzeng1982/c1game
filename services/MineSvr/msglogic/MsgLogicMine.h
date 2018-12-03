#pragma once
#include "MsgBase_c.h"
#include "ss_proto.h"


class CSsMineGetInfoReq : public IMsgBase_c
{
public:
    virtual int HandleServerMsg(PKGMETA::SSPKG* pstSsPkg);
};


class CSsMineExploreReq : public IMsgBase_c
{
public:
    virtual int HandleServerMsg(PKGMETA::SSPKG* pstSsPkg);
};

class CSsMineDealOreReq : public IMsgBase_c
{
public:
    virtual int HandleServerMsg(PKGMETA::SSPKG* pstSsPkg);
};


class CSsMineGetAwardReq : public IMsgBase_c
{
public:
    virtual int HandleServerMsg(PKGMETA::SSPKG* pstSsPkg);
};

class CSsMineGetRevengerInfoReq : public IMsgBase_c
{
public:
    virtual int HandleServerMsg(PKGMETA::SSPKG* pstSsPkg);
};

class CSsMineFightResultReq : public IMsgBase_c
{
public:
    virtual int HandleServerMsg(PKGMETA::SSPKG* pstSsPkg);
};

class CSsMineSvrCombineUptNtf : public IMsgBase_c
{
public:
	virtual int HandleServerMsg(PKGMETA::SSPKG* pstSsPkg);
};

//  回复
class CSsMineGetOreDataRsp : public IMsgBase_c
{
public:
    virtual int HandleServerMsg(PKGMETA::SSPKG* pstSsPkg);
};

class CSsMineGetPlayerDataRsp : public IMsgBase_c
{
public:
    virtual int HandleServerMsg(PKGMETA::SSPKG* pstSsPkg);
};
