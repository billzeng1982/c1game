#include "GuildTransaction.h"
#include "LogMacros.h"
#include "../module/Guild/GuildMgr.h"
#include "../module/Player/PlayerMgr.h"
#include "../framework/GuildSvrMsgLayer.h"
#include "../module/Fight/GuildFightMgr.h"
#include "GameTime.h"
#include "../module/Guild/GuildBossMgr.h"

using namespace PKGMETA;

/*----------------------------------------------------------------------------------------------------*/

void GetGuildAction::Reset()
{
    IAction::Reset();
    m_ullGuildId = 0;
    m_pObjTrans = NULL;
}

int GetGuildAction::Execute(Transaction* pObjTrans)
{
    assert(pObjTrans != NULL);
    m_pObjTrans = (GuildTransaction*)pObjTrans;
    TActionToken ullToken = this->GetToken();

    if (m_ullGuildId == 0)
    {
        if (m_pObjTrans->GetGuildId() == 0)
        {
            this->SetFiniFlag(1);
            return 1;
        }
        else
        {
            m_ullGuildId = m_pObjTrans->GetGuildId();
        }
    }

    Guild* poGuild = GuildMgr::Instance().GetGuild(m_ullGuildId, ullToken);
    if (poGuild == NULL)
    {
        //异步
        return 0;
    }
    else
    {
        this->SetFiniFlag(1);
        m_pObjTrans->SetGuild(poGuild);
        return 1;
    }
}

void GetGuildAction::OnAsyncRspMsg(TActionIndex iActionIdx, const void* pResult, unsigned int dwResLen)
{
   assert( iActionIdx == m_iIndex );
   m_pObjTrans->SetGuild((Guild*)pResult);
   this->SetFiniFlag(1);  // 表示动作执行完成
}




/*----------------------------------------------------------------------------------------------------*/

void GetGuildByNameAction::Reset()
{
    IAction::Reset();
    m_pszName = NULL;
    m_pObjTrans = NULL;
}

int GetGuildByNameAction::Execute(Transaction* pObjTrans)
{
    assert(pObjTrans != NULL);
    m_pObjTrans = (GuildTransaction*)pObjTrans;
    TActionToken ullToken = this->GetToken();

    Guild* poGuild = GuildMgr::Instance().GetGuild(m_pszName, ullToken);
    if (poGuild == NULL)
    {
        return 0;
    }
    else
    {
        this->SetFiniFlag(1);
        m_pObjTrans->SetGuild(poGuild);
        return 1;
    }
}

void GetGuildByNameAction::OnAsyncRspMsg(TActionIndex iActionIdx, const void* pResult, unsigned int dwResLen)
{
   assert( iActionIdx == m_iIndex );
   m_pObjTrans->SetGuild((Guild*)pResult);
   this->SetFiniFlag(1);  // 表示动作执行完成
}



/*----------------------------------------------------------------------------------------------------*/


void CreateGuildAction::Reset()
{
    IAction::Reset();
    m_szGuildName[0] = '\0';
    m_pObjTrans = NULL;
}

int CreateGuildAction::Execute(Transaction* pObjTrans)
{
    assert(pObjTrans!=NULL);
    m_pObjTrans = (GuildTransaction*)pObjTrans;
    TActionToken ullToken = this->GetToken();

    //没有找到玩家信息的情况下不去做后面的处理
    Player* poPlayer = m_pObjTrans->GetPlayer();
    if (poPlayer == NULL)
    {
        LOGERR_r("Create Guild failed, Player not found");
        this->SetFiniFlag(1);
        return 1;
    }

    //GuildId不为0，说明已有公会，不能创建
    if (poPlayer->GetGuildId() != 0)
    {
        m_pObjTrans->SetErrNo(ERR_ALREADY_HAVE_GUILD);
        this->SetFiniFlag(1);
        return 1;
    }

    int iRet = GuildMgr::Instance().CreateGuild(m_szGuildName, ullToken);
    if (iRet == ERR_NONE)
    {
        return 0;
    }
    //iRet不为ERR_NONE,说明创建失败,直接返回,不用等异步结果
    else
    {
        this->SetFiniFlag(1);
        m_pObjTrans->SetErrNo(iRet);
        return 1;
    }
}

void CreateGuildAction::OnAsyncRspMsg(TActionIndex iActionIdx, const void* pResult, unsigned int dwResLen)
{
    assert( iActionIdx == m_iIndex );
    m_pObjTrans->SetGuild((Guild*)pResult);

    this->SetFiniFlag(1);
}




/*----------------------------------------------------------------------------------------------------*/


void GetPlayerAction::Reset()
{
    IAction::Reset();
    m_ullPlayerId = 0;
    m_pObjTrans = NULL;
}

int GetPlayerAction::Execute(Transaction* pObjTrans)
{
    assert(pObjTrans != NULL);
    m_pObjTrans = (GuildTransaction*)pObjTrans;
    TActionToken ullToken = this->GetToken();

    //PlayerId不能为0
    if (m_ullPlayerId == 0)
    {
        LOGERR_r("Guild PlayerId is 0, execute failed");
        this->SetFiniFlag(1);
        return 1;
    }

    Player* poPlayer = PlayerMgr::Instance().GetPlayer(m_ullPlayerId, ullToken);
    //需要等待异步执行结果
    if (poPlayer == NULL)
    {
        return 0;
    }
    //数据在内存中，不用等待异步执行结果
    else
    {
        m_pObjTrans->SetPlayer(poPlayer);
        this->SetFiniFlag(1);
        return 1;
    }
}

void GetPlayerAction::OnAsyncRspMsg(TActionIndex iActionIdx, const void* pResult, unsigned int dwResLen)
{
   assert( iActionIdx == m_iIndex );
   Player* poPlayer = (Player*)pResult;

   m_pObjTrans->SetPlayer(poPlayer);
   m_pObjTrans->SetGuildId(poPlayer->GetGuildId());
   this->SetFiniFlag(1);  // 表示动作执行完成
}




/*----------------------------------------------------------------------------------------------------*/


void GuildTransaction::Reset()
{
    Transaction::Reset();
    bzero(&m_stSsReqPkg, sizeof(m_stSsReqPkg));
    m_poGuild = NULL;
    m_poPlayer = NULL;
    m_ullGuildId = 0;
    m_iErrNo = ERR_NONE;
}

//  军团创建初始化处理
void GuildTransaction::_HandleGuildCreateReq()
{
    SSPKG stSSRspPkg = {0};
    stSSRspPkg.m_stHead.m_wMsgId = SS_MSG_CREATE_GUILD_RSP;
    stSSRspPkg.m_stHead.m_ullUin = m_stSsReqPkg.m_stHead.m_ullUin;

    SS_PKG_CREATE_GUILD_RSP& rstSsPkgRsp = stSSRspPkg.m_stBody.m_stCreateGuildRsp;
    SS_PKG_CREATE_GUILD_REQ& rstSsPkgReq = m_stSsReqPkg.m_stBody.m_stCreateGuildReq;
    rstSsPkgRsp.m_nErrNo = ERR_NONE;

    do
    {
        if (m_iErrNo != ERR_NONE)
        {
            rstSsPkgRsp.m_nErrNo = m_iErrNo;
            break;
        }

        if (m_poPlayer==NULL || m_poGuild == NULL)
        {
            rstSsPkgRsp.m_nErrNo = ERR_EXIST_GUILDNAME;
            break;
        }
        m_poGuild->NewInit();
        m_poPlayer->SetGuildId(m_poGuild->GetGuildId());

        rstSsPkgReq.m_stLeaderInfo.m_bIdentity = GUILD_IDENTITY_LEADER;
		rstSsPkgReq.m_stLeaderInfo.m_bSalaryIdentityToday = GUILD_IDENTITY_LEADER;
        rstSsPkgReq.m_stLeaderInfo.m_ullJoinGuildTimeStap = CGameTime::Instance().GetCurrTimeMs();
        rstSsPkgReq.m_stLeaderInfo.m_ullLastLogoutTimeStap = CGameTime::Instance().GetCurrTimeMs();

        m_poGuild->SetLeader(rstSsPkgReq.m_stLeaderInfo);
        m_poGuild->AddMember(rstSsPkgReq.m_stLeaderInfo, &rstSsPkgReq.m_stLeaderInfo);

        m_poGuild->PackGuildWholeData(rstSsPkgRsp.m_stGuildWholeData, (uint16_t)m_stSsReqPkg.m_stHead.m_ullReservId);
		m_poGuild->UploadGuildExpeditionInfo();
        rstSsPkgRsp.m_stFightStateInfo = m_poGuild->GetGuildFightState();
        GuildBossMgr::Instance().GetState(rstSsPkgRsp.m_stGuildBossInfo);
    }while(false);

    GuildSvrMsgLayer::Instance().SendToZoneSvr(stSSRspPkg);
    return;
}

void GuildTransaction::OnFinished(int iErrCode)
{
    uint16_t wMgsId = m_stSsReqPkg.m_stHead.m_wMsgId;

    if (wMgsId == SS_MSG_CREATE_GUILD_REQ)
    {
        _HandleGuildCreateReq();
        return;
    }

    if (wMgsId == SS_MSG_GUILD_PLAYER_RESET)
    {
        if (m_poPlayer != NULL)
        {
            m_poPlayer->SetGuildId(0);
        }
        return;
    }

    IMsgBase* poMsgHandler = GuildSvrMsgLayer::Instance().GetServerMsgHandler(wMgsId);
    if( !poMsgHandler )
    {
        LOGERR_r("Can not find msg handler. id <%u>", wMgsId);
        return;
    }

    int i = 0;
    poMsgHandler->HandleServerMsg(m_stSsReqPkg, (void*)&i);

    return;
}


/*----------------------------------------------------------------------------------------------------*/
//Gm操作  对公会信息进行更新

void GuildGmUpdateInfoAction::Reset()
{
    IAction::Reset();
    m_ullGuildId = 0;
    m_pObjTrans = NULL;
}

int GuildGmUpdateInfoAction::Execute(Transaction* pObjTrans)
{
    assert(pObjTrans != NULL);
    m_pObjTrans = (GuildTransaction*)pObjTrans;
    TActionToken ullToken = this->GetToken();
    LOGWARN_r("Gm Action execute <%lu> ", m_ullGuildId);
    if (m_ullGuildId == 0)
    {
        LOGRUN_r("Guildid is 0, tokenid=%lu", ullToken);
        this->SetFiniFlag(1);
        return 1;
    }

    Guild* poGuild = GuildMgr::Instance().GetGuild(m_ullGuildId, ullToken);
    //需要等待
    if (poGuild == NULL)
    {
        return 0;
    }
    else
    {
        this->SetFiniFlag(1);
        m_pObjTrans->SetGuild(poGuild);
        return 1;
    }
}

void GuildGmUpdateInfoAction::OnAsyncRspMsg(TActionIndex iActionIdx, const void* pResult, unsigned int dwResLen)
{
    assert( iActionIdx == m_iIndex );
    m_pObjTrans->SetGuild((Guild*)pResult);

    this->SetFiniFlag(1);
}

