#pragma once

#include "MsgBase.h"
#include "common_proto.h"

class AsyncpvpStartReq_CS :  public IMsgBase
{
public:
    virtual int HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL );
};

class AsyncpvpSettleReq_CS :  public IMsgBase
{
public:
    virtual int HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL );
};

class AsyncpvpRefreshOpponentReq_CS : public IMsgBase
{
public:
    virtual int HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL );
};

class AsyncpvpGetDataReq_CS :  public IMsgBase
{
public:
    virtual int HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL );
};

class AsyncpvpChgTeamReq_CS :  public IMsgBase
{
public:
    virtual int HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL );
};

class AsyncpvpScoreRewardReq_CS :  public IMsgBase
{
public:
    virtual int HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL );
};

class AsyncpvpRankRewardReq_CS :  public IMsgBase
{
public:
    virtual int HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL );
};

class AsyncpvpFightTimesBuyReq_CS :  public IMsgBase
{
public:
    virtual int HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL );
};

class AsyncpvpGetTopListReq_CS : public IMsgBase
{
public:
    virtual int HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL );
};

class AsyncpvpWorshipReq_CS : public IMsgBase
{
public:
	virtual int HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL );
};

class AsyncpvpGetWorshipGoldReq_CS : public IMsgBase
{
public:
	virtual int HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL );
};

class AsyncpvpResetCdReq_CS :  public IMsgBase
{
public:
	virtual int HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL );
};

class AsyncpvpSkipFightReq_CS : public IMsgBase
{
public:
	virtual int HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL );
};

class AsyncpvpGetPlayerInfoReq_CS :  public IMsgBase
{
public:
	virtual int HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL );
};


//SS–≠“È
class AsyncpvpStartRsp_SS : public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class AsyncpvpSettleRsp_SS : public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class AsyncpvpRefreshOpponentRsp_SS : public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class AsyncpvpGetDataRsp_SS : public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class AsyncpvpRefreshTopListNtf_SS : public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class AsyncpvpGetWorshipGoldRsp_SS : public IMsgBase
{
public:
	virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class AsyncpvpGetPlayerInfoRsp_SS : public IMsgBase
{
public:
	virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

