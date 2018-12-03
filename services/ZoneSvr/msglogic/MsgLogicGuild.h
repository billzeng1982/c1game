#pragma once

#include "MsgBase.h"
#include "common_proto.h"

//CS协议
class JoinGuildReq_CS :  public IMsgBase
{
public:
    virtual int HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL );
};

class QuitGuildReq_CS :  public IMsgBase
{
public:
    virtual int HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL );
};

class GuildSetNeedApplyReq_CS : public IMsgBase
{
public:
    virtual int HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL );
};

class GuildSetLevelLimitReq_CS : public IMsgBase
{
public:
	virtual int HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL );
};

class CreateGuildReq_CS :  public IMsgBase
{
public:
    virtual int HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL );
};

class DissolveGuildReq_CS :  public IMsgBase
{
public:
    virtual int HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL );
};

class RefreshGuildListReq_CS :  public IMsgBase
{
public:
    virtual int HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL );
};

class GetGuildDetailReq_CS :  public IMsgBase
{
public:
    virtual int HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL );
};

class GuildDealApplyReq_CS :  public IMsgBase
{
public:
    virtual int HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL );
};

class UpdateGulidNotice_CS :  public IMsgBase
{
public:
    virtual int HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL );
};

class SetGuildMemJob_CS :  public IMsgBase
{
public:
    virtual int HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL );
};

class GuildKickPlayer_CS :  public IMsgBase
{
public:
    virtual int HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL );
};

class GuildGetTask_CS :  public IMsgBase
{
public:
    virtual int HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL );
};

class GuildLvUpReq_CS :  public IMsgBase
{
public:
    virtual int HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL );
};

class SearchGuildReq_CS :  public IMsgBase
{
public:
    virtual int HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL );
};

//class GuildPurchaseReq_CS :  public IMsgBase
//{
//public:
//    virtual int HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL );
//};

class GuildDonateReq_CS :  public IMsgBase
{
public:
    virtual int HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL );
};

class GuildFightApplyReq_CS :  public IMsgBase
{
public:
    virtual int HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL );
};

class GuildFightGetApplyListReq_CS :  public IMsgBase
{
public:
    virtual int HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL );
};

class GuildFightGetAgainstInfoReq_CS :  public IMsgBase
{
public:
    virtual int HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL );
};

class GuildFightArenaJoinReq_CS :  public IMsgBase
{
public:
    virtual int HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL );
};

class GuildFightArenaQuitReq_CS :  public IMsgBase
{
public:
    virtual int HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL );
};

class GuildFightArenaMoveReq_CS :  public IMsgBase
{
public:
    virtual int HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL );
};

class GuildFightQuitMatchReq_CS :  public IMsgBase
{
public:
    virtual int HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL );
};

class GuildFightCancleGodReq_CS :  public IMsgBase
{
public:
    virtual int HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL );
};

class GuildGetGuildRankReq_CS :  public IMsgBase
{
public:
    virtual int HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL );
};

class GuildGetGFightRankReq_CS :  public IMsgBase
{
public:
    virtual int HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL );
};

class GuildResetMarketOrTaskReq_CS :  public IMsgBase
{
public:
    virtual int HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL );
};

class GuildActivityReq_CS :  public IMsgBase
{
public:
    virtual int HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL );
};

class GuildBossResetReq_CS :  public IMsgBase
{
public:
    virtual int HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL );
};

class GuildBossGetKilledAwardReq_CS :  public IMsgBase
{
public:
    virtual int HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL );
};

class GuildBossCompetitorInfoReq_CS : public IMsgBase
{
public:
    virtual int HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL);
};

class GuildBossFightTimesReq_CS : public IMsgBase
{
public:
    virtual int HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /* = NULL */);
};

class GuildBossSingleRewardReq_CS : public IMsgBase
{
public:
    virtual int HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara);
};

class GuildBossDamageRankInGuildReq_CS : public IMsgBase
{
public:
    virtual int HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /* = NULL */);
};

class GuildBossPassedGuildList_CS : public IMsgBase
{
public:
    virtual int HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /* = NULL */);
};

class GuildHangSettleReq_CS : public IMsgBase
{
public:
	virtual int HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL);
};

class GuildHangPurchaseSlotReq_CS : public IMsgBase
{
public:
	virtual int HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL);
};

class GuildHangLayGeneralReq_CS : public IMsgBase
{
public:
	virtual int HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL);
};

class GuildHangSpeedPartnerReq_CS : public IMsgBase
{
public:
	virtual int HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL);
};

class GuildHangStarReq_CS : public IMsgBase
{
public:
    virtual int HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL);
};

class GuildBossGetSubRwdMemListReq_CS :public IMsgBase
{
public:
    virtual int HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL);
};


//SS协议
class CreateGuildRsp_SS : public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class JoinGuildRsp_SS : public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class GuildSetNeedApplyRsp_SS : public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class GuildSetLevelLimitRsp_SS : public IMsgBase
{
public:
	virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class DissolveGuildRsp_SS : public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class QuitGuildRsp_SS : public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class RefreshGuildListRsp_SS : public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class GetGuildDetailRsp_SS : public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class GuildDealApplyRsp_SS : public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class GuildBroadCastNtf_SS :  public IMsgBase
{
public:
   virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class UpdateGulidNotice_SS :  public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class SetGuildMemJob_SS :  public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class GuildKickPlayerRsp_SS :  public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class SearchGuildRsp_SS :  public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class GuildLvUpRsp_SS :  public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class GuildDrawSalaryRsp_SS : public IMsgBase
{
public:
	virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};


class GuildFightApplyRsp_SS :  public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class GuildFightApplyListNtf_SS :  public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class GuildFightAgainstNtf_SS :  public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class GuildFightArenaJoinRsp_SS :  public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class GuildFightArenaQuitRsp_SS :  public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class GuildFightArenaMoveRsp_SS :  public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class GuildFightQuitMatchRsp_SS :  public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class GuildFightCancleGodRsp_SS :  public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};


class GuildFightPvpMatchReq_SS :  public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class GuildFightMsgBroad_SS :  public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class GuildFightSettleNtf_SS :  public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class GuildGetGuildRankRsp_SS :  public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class GuildGetGFightRankRsp_SS :  public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class GuildUploadReplayRsp_SS :  public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class GuildBossEnterFightRsp_SS :  public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class GuildBossCompetitorInfoRsp_SS : public IMsgBase
{
public:
    virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL);
};

class GuildBossDamageRankInGuildRsp_SS : public IMsgBase
{
public:
    virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /* = NULL */);
};

class GuildGetMemberListRsp_SS: public IMsgBase
{
public:
    virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL /* = NULL */);
};

class GuildBossGetPassedListRsp_SS: public IMsgBase
{
public:
    virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL /* = NULL */);
};

class GuildSpeedPartnerRsp_SS: public IMsgBase
{
public:
	virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL /* = NULL */);
};

class GuildGetBeSpeededInfoRsp_SS: public IMsgBase
{
public:
	virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL /* = NULL */);
};

class GuildHangStarRsp_SS: public IMsgBase
{
public:
    virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL /* = NULL */);
};

class GuildBossGetMemFightTimesRsp_SS :public IMsgBase
{
public:
    virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL /* = NULL */);
};

class GuildBossGetMemWhoGetSubRwdRsp_SS :public IMsgBase
{
public:
    virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL /* = NULL */);
};
