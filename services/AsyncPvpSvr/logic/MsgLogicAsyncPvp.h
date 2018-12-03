#pragma once
#include "MsgBase.h"
#include "ss_proto.h"

class AsyncPvpStartReq : public IMsgBase
{
public:
	virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class AsyncPvpSettleReq : public IMsgBase
{
public:
	virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class AsyncPvpRefreshOpponentReq : public IMsgBase
{
public:
	virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class AsyncPvpGetDataReq : public IMsgBase
{
public:
	virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class AsyncPvpUptPlayerNtf : public IMsgBase
{
public:
	virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class AsyncPvpWorshippedNtf : public IMsgBase
{
public:
	virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class AsyncPvpGetWorshipGold : public IMsgBase
{
public:
	virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class AsyncPvpGetPlayerInfo : public IMsgBase
{
public:
	virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

