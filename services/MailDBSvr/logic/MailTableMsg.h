#pragma once
#include "MsgBase_r.h"

/*******************************************
 *  玩家邮件箱处理
 *******************************************/
class CSsMailPriTableCreateReq : public IMsgBase_r
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class CSsMailPriTableGetDataReq : public IMsgBase_r
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class CSsMailPriTableUptNtf : public IMsgBase_r
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};


/*******************************************
 *  公共邮件处理
 *******************************************/

class CSsMailPubTableGetDataReq : public IMsgBase_r
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class CSsMailPubTableDelNtf : public IMsgBase_r
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class CSsMailPubTableAddReq : public IMsgBase_r
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};


