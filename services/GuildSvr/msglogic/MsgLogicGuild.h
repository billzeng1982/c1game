#pragma once

#include "msglayer/MsgBase.h"
#include "common_proto.h"
#include "../module/Guild/Guild.h"
#include "../module/Player/Player.h"

class GuildDetailsReq_SS :  public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );

private:
    void _HandleGuildDetailsReq(PKGMETA::SSPKG& rstSsPkg, Guild* poGuild, Player* poPlayer);

};

class RefreshGuildListReq_SS :  public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class CreateGuildReq_SS :  public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class DissolveGuildReq_SS :  public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );

private:
    void _HandleGuildDissolveReq(PKGMETA::SSPKG& rstSsPkg, Guild* poGuild);
};

class GuildSetNeedApplyReq_SS : public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
private:
    void _HandleSetNeedApplyReq(PKGMETA::SSPKG& rstSsPkg, Guild* poGuild);
};

class GuildSetLevelLimitReq_SS : public IMsgBase
{
public:
	virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
private:
	void _HandleSetLevelLimitReq(PKGMETA::SSPKG& rstSsPkg, Guild* poGuild);
};

class JoinGuildReq_SS :  public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );

private:
    void _HandleGuildJoinReq(PKGMETA::SSPKG& rstSsPkg, Guild* poGuild, Player* poPlayer);
};

class QuitGuildReq_SS :  public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );

private:
    void _HandleGuildQuitReq(PKGMETA::SSPKG& rstSsPkg, Guild* poGuild, Player* poPlayer);
};

class GuildDealApplyReq_SS :  public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );

private:
    void _HandleGuildDealApplyReq(PKGMETA::SSPKG& rstSsPkg, Guild* poGuild, Player* poPlayer);
};

class RefreshMemInfoNtf_SS :  public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );

private:
    void _HandleGuildRefreshMemNtf(PKGMETA::SSPKG& rstSsPkg, Guild* poGuild);
};

class GuildLvUpReq_SS :  public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );

private:
    void _HandleGuildLvUpReq(PKGMETA::SSPKG& rstSsPkg, Guild* poGuild);
};

class UpdateGulidNotice_SS :  public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );

private:
    void _HandleGuildUptNoticeReq(PKGMETA::SSPKG& rstSsPkg, Guild* poGuild);
};

class GulidUptFundNtf_SS :  public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );

private:
    void _HandleGuildUptFundNtf(PKGMETA::SSPKG& rstSsPkg, Guild* poGuild);
};

class SearchGuildReq_SS :  public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
private:
	void _HandleSearchGuildReq(PKGMETA::SSPKG& rstSsPkg, Guild* poGuild);
};

class SetGuildMemJob_SS :  public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );

private:
    void _HandleGuildSetMemJobReq(PKGMETA::SSPKG& rstSsPkg, Guild* poGuild);
};

class GuildKickPlayerReq_SS :  public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );

private:
    void _HandleGuildKickPlayerReq(PKGMETA::SSPKG& rstSsPkg, Guild* poGuild, Player* poPlayer);
};

class GFightApplyReq_SS :  public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );

private:
    void _HandleGuildFightApplyReq(PKGMETA::SSPKG& rstSsPkg, Guild* poGuild);
};

class GuildMessageSendReq_SS :  public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );

private:
    void _HandleMessageSendReq(PKGMETA::SSPKG& rstSsPkg, Guild* poGuild);
};

class GetGuildRankReq_SS :  public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class GetGFightRankReq_SS :  public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class RefreshRoomNtf_SS :  public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );

private:
    void _HandleGuildRefreshRoomNtf(PKGMETA::SSPKG& rstSsPkg, Guild* poGuild);
};

class GuildGmUpdateInfoReq_SS : public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class GuildUploadReplayReq_SS : public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};



class GulidBossGetInfoReq_SS : public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class GulidBossResetReq_SS : public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
private:
    void _HandleBossResetReq(PKGMETA::SSPKG& rstSsPkg, Guild* poGuild);
};

class GulidBossFightSettleNtf_SS : public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
private:
    void _HandleBossFightSettleNtf(PKGMETA::SSPKG& rstSsPkg, Guild* poGuild);
};

class GuildBossEnterFightReq_SS : public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
private:
    void _HandleBossEnterFightReq(PKGMETA::SSPKG& rstSsPkg, Guild* poGuild);
};


class GuildLevelEventNtf_SS : public IMsgBase
{
public:
	virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
private:
	void _HandleLevelEventNtf(PKGMETA::SSPKG& rstSsPkg, Guild* poGuild);
};


class GuildDrawSalaryReq_SS : public IMsgBase
{
public:
	virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
private:
	void _HandleDrawSalaryReq(PKGMETA::SSPKG& rstSsPkg, Guild* poGuild);
};

//class GuildCompetitorInfoReq_SS : public IMsgBase
//{
//public:
//    virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL/* = NULL */);
//private:
//    void _HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, Guild* poGuild, Player* poPlayer);
//};

class GuildBossDamageRankInGuildReq_SS : public IMsgBase
{
public:
    virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL /* = NULL */);
};

//class GuildBossGetRankListRsp_SS: public IMsgBase
//{
//public:
//    virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL /* = NULL */);
//};

class GuildGetMemberListReq_SS:public IMsgBase
{
public:
    virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL /* = NULL */);
};


class GuildBossGetPassedListReq_SS:public IMsgBase
{
public:
    virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL /* = NULL */);
};
class GuildSpeedPartnerReq_SS:public IMsgBase
{
public:
	virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL /* = NULL */);
private:
	void _HandleSpeedPartnerReq(PKGMETA::SSPKG& rstSsPkg, Guild* poGuild);
};

class GuildGetBeSpeededInfoReq_SS:public IMsgBase
{
public:
	virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL /* = NULL */);
private:
	void _HandleGetBeSpeededInfoReq(PKGMETA::SSPKG& rstSsPkg, Guild* poGuild);
};

class GuildHangStarReq_SS:public IMsgBase
{
public:
    virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL /* = NULL */);
private:
    void _HandleGuildHangStarReq(PKGMETA::SSPKG& rstSsPkg, Guild* poGuild);
};

class GuildBossGetMemFightTimesReq_SS :public IMsgBase
{
public:
    virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL /* = NULL */);
};

class GuildBossSetBossFightTimesToZeroNtf_SS :public IMsgBase
{
public:
    virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL /* = NULL */);
};

class GuildBossGetMemWhoGetSubRwdReq_SS :public IMsgBase
{
public:
    virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL /* = NULL */);
};

class GuildExpeditionCommonNtf_SS :public IMsgBase
{
public:
	virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL /* = NULL */);
private:
	void _HandlExpeditionCommonNtf(PKGMETA::SSPKG& rstSsPkg, Guild* poGuild);
};

