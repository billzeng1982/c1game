#pragma once
#include "MsgBase.h"
#include "common_proto.h"

class CSsFriendHandleReq : public IMsgBase
{
public:
	virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};


class CSsFriendGetListReq : public IMsgBase
{
public:
	virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class CSsFriendSearchReq : public IMsgBase
{
public:
	virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};
class CSsFriendEventNtf : public IMsgBase
{
public:
	virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class CSsFriendSendApReq : public IMsgBase
{
public:
	virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class CSsFriendGetApReq : public IMsgBase
{
public:
	virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class CSsFriendChangeNameReq : public IMsgBase
{
public:
	virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};
