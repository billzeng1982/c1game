#include "strutil.h"
#include "ObjectUpdatorMgr.h"
#include "FakeRandom.h"
#include "LogMacros.h"
#include "MsgLogicLogin.h"
#include "../framework/ZoneSvrMsgLayer.h"
#include "../module/player/PlayerMgr.h"
#include "../module/player/PlayerLogic.h"
#include "../module/player/PlayerStateMachine.h"
#include "../module/Majesty.h"
#include "../module/FightPVE.h"
#include "../module/Mail.h"
#include "../../module/player/PlayerStateMachine.h"
#include "ZoneSvr.h"
#include "ZoneLog.h"
#include "dwlog_svr.h"
#include "../module/SvrTime.h"
#include "../module/Tutorial.h"

using namespace PKGMETA;
using namespace DWLOG;

#define SDKLOGIN_SWITCH_OFF 0
#define SDKLOGIN_SWITCH_TEST 2

//是否含有特殊字符
bool IsContainSpecialChar(char* szName)
{
    for (int i = 0; i < (int)strlen(szName); i++)
    {
        if (szName[i] == '|' || szName[i] == '*' || szName[i] == '%' || szName[i] == ' ' || szName[i] == '\t')
        {
            return true;
        }
    }

    return false;
}

bool AccountLoginReq_CS::_CheckNameVaild(char* szName)
{
    int offset = 0;
    while (*(szName + offset) != '\0')
    {
        if (offset >= PKGMETA::MAX_NAME_LENGTH)
        {
            return false;
        }

        if ((*(szName + offset) >= 'a' && *(szName + offset) <= 'z')
            || (*(szName + offset) >= 'A' && *(szName + offset) <= 'Z')
            || (*(szName + offset) >= '0' && *(szName + offset) <= '9')
            || (*(szName + offset) == '_')
            || (*(szName + offset) == '.'))
        {
            offset++;
            continue;
        }
        else
        {
            return false;
        }
    }

    return true;
}

int AccountLoginReq_CS::_HandleAccReconn(const PKGMETA::CONNSESSION* pstSession, Player* poPlayer, PKGMETA::CSPKG& rstCsPkg)
{
    assert(poPlayer);
    SC_PKG_ACCOUNT_LOGIN_RSP& rstAccountLoginRsp = m_stScPkg.m_stBody.m_stAccountLoginRsp;
	//CS_PKG_ACCOUNT_LOGIN_REQ& rstAccountLoginReq = rstCsPkg.m_stBody.m_stAccountLoginReq;
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_ACCOUNT_LOGIN_RSP;

    //处于等待重连状态
    if (poPlayer->IsCurState(PLAYER_STATE_RECONN_WAIT) ||
      (poPlayer->IsCurState(PLAYER_STATE_INGAME) && strcmp(poPlayer->GetMacAddr(), poPlayer->GetMacAddr()) == 0))
    {
        LOGRUN("Player(%lu) Login, but in Reconn_Wait state, Reconn", poPlayer->GetUin());

        //更新session之前给之前登录的玩家发送一个你被挤掉了的数据包
        SCPKG m_stScRPkg;
        PKGMETA::CONNSESSION* ExistedPlayerSession = poPlayer->GetConnSession();
        m_stScRPkg.m_stHead.m_wMsgId = SC_MSG_REPLACE_DUP_ACC_NTF;
        m_stScRPkg.m_stBody.m_stReplaceDupAccNtf.m_chPad = 1;
        ZoneSvrMsgLayer::Instance().SendToClient(ExistedPlayerSession, &m_stScRPkg);

        //更新session
        PlayerMgr::Instance().DelFromPlayerSessMap(poPlayer);
        poPlayer->SetConnSession(pstSession);
        PlayerMgr::Instance().AddToPlayerSessMap(poPlayer);

        //更新登录时间
        poPlayer->SetLastLoginTime(CGameTime::Instance().GetCurrSecond());

        poPlayer->GetConnSession()->m_chCmd = CONNSESSION_CMD_INPROC;
        rstAccountLoginRsp.m_nErrNo = ERR_NONE;
        rstAccountLoginRsp.m_ullUin = poPlayer->GetUin();
        rstAccountLoginRsp.m_dwOnlineTokenId = poPlayer->GetOnlineTokenId();
        rstAccountLoginRsp.m_ullTimeStamp = (uint64_t)CGameTime::Instance().GetCurrSecond();

        rstAccountLoginRsp.m_ullServerOpenTimeStamp = SvrTime::Instance().GetOpenSvrTime();
        StrCpy(rstAccountLoginRsp.m_szAccountName, poPlayer->GetAccountName(), PKGMETA::MAX_NAME_LENGTH);
        StrCpy(rstAccountLoginRsp.m_stSeverConfig.m_szUrl, ZoneSvr::Instance().GetConfig().m_szFeedBackUrl, MAX_LEN_URL);
        rstAccountLoginRsp.m_stSeverConfig.m_chSerialNumSwitch = ZoneSvr::Instance().GetConfig().m_iSerialNumSwitch;
        m_stScPkg.m_stBody.m_stAccountLoginRsp.m_stSeverConfig.m_chTestPaySwitch = ZoneSvr::Instance().GetConfig().m_iTestPaySwitch;
		//功能开关
		m_stScPkg.m_stBody.m_stAccountLoginRsp.m_stSeverConfig.m_llCommonSwitch =
			(int64_t)ZoneSvr::Instance().GetConfig().m_iGuildFightSwitch << COMMON_SWITCH_TYPE_GUILD_FIGHT
			| (int64_t)ZoneSvr::Instance().GetConfig().m_iMineSwitch << COMMON_SWITCH_TYPE_MINE;
        ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

        //切换为重连登录状态
        PlayerStateMachine::Instance().ChangeState(poPlayer, PLAYER_STATE_RECONN_LOGIN);
    }
    //不允许顶号
    else
    {
        /*rstAccountLoginRsp.m_nErrNo = ERR_ALREADY_EXISTED;
        ZoneSvrMsgLayer::Instance().SendToClient(pstSession, &m_stScPkg, PKGMETA::CONNSESSION_CMD_RELOGIN);*/
        SCPKG m_stScRPkg;
        PKGMETA::CONNSESSION* ExistedPlayerSession = poPlayer->GetConnSession();
        m_stScRPkg.m_stHead.m_wMsgId = SC_MSG_REPLACE_DUP_ACC_NTF;
        m_stScRPkg.m_stBody.m_stReplaceDupAccNtf.m_chPad = 111;
        ZoneSvrMsgLayer::Instance().SendToClient(ExistedPlayerSession, &m_stScRPkg);
        //LOGERR("account %s login failed, because this player is already existed.", poPlayer->GetAccountName());
    }

    return 1;
}
//非Sdk登录
int AccountLoginReq_CS::_HandleConnect(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg)
{
    PlayerMgr& roPlayerMgr = PlayerMgr::Instance();
    CS_PKG_ACCOUNT_LOGIN_REQ& rstAccountLoginReq = rstCsPkg.m_stBody.m_stAccountLoginReq;
    SC_PKG_ACCOUNT_LOGIN_RSP& rstAccountLoginRsp = m_stScPkg.m_stBody.m_stAccountLoginRsp;
    //m_stScPkg.m_stBody.m_stAccountLoginRsp.m_ullServerOpenTimeStamp;
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_ACCOUNT_LOGIN_RSP;

    Player* poPlayer = roPlayerMgr.GetPlayerBySdkUserNameAndSvrId(rstAccountLoginReq.m_szAccountName, rstAccountLoginReq.m_dwLoginSvrId);
    if (poPlayer)
    {
        _HandleAccReconn(pstSession, poPlayer, rstCsPkg);
        return 0;
    }

    // 正常登陆流程
    poPlayer = roPlayerMgr.NewPlayer();
    if (!poPlayer)
    {
        rstAccountLoginRsp.m_nErrNo = ERR_BAD_ALLOC;
        ZoneSvrMsgLayer::Instance().SendToClient(pstSession, &m_stScPkg, PKGMETA::CONNSESSION_CMD_STOP);
        LOGERR("account %s create new player failed.", rstAccountLoginReq.m_szAccountName);
        return -1;
    }
	//先设置SvrId
	poPlayer->SetLoginSvrId(rstAccountLoginReq.m_dwLoginSvrId);
	//AccountName需要用到SvrId来合成
    poPlayer->SetAccountName(rstAccountLoginReq.m_szAccountName);
	poPlayer->SetSdkUserName(rstAccountLoginReq.m_szAccountName);
    poPlayer->SetMacAddr(rstAccountLoginReq.m_szMacAddress);
    poPlayer->SetPhoneType(rstAccountLoginReq.m_szPhoneType);
    poPlayer->SetOSType(rstAccountLoginReq.m_szOSType);
    poPlayer->SetLoginType(rstAccountLoginReq.m_bLoginType);
	
    poPlayer->SetConnSession(pstSession);
    roPlayerMgr.AddToPlayerSessMap(poPlayer);
    roPlayerMgr.AddToAccountNameHM(poPlayer);
    PlayerStateMachine::Instance().ChangeState(poPlayer, PLAYER_STATE_LOGIN);

    // 设置随机在线token,用于断线重连
    poPlayer->SetOnlineTokenId(CFakeRandom::Instance().Random());
    poPlayer->SetLastUptDbTime(CGameTime::Instance().GetCurrSecond());
    poPlayer->SetLastLoginTime(CGameTime::Instance().GetCurrSecond());

    // 设置客户端最高版本号
    poPlayer->SetProtocolVersion(rstAccountLoginReq.m_wVersion);

    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_ACCOUNT_LOGIN_REQ;
	m_stSsPkg.m_stBody.m_stAccountLoginReq.m_dwLoginSvrId = rstAccountLoginReq.m_dwLoginSvrId;
	StrCpy(m_stSsPkg.m_stBody.m_stAccountLoginReq.m_szChannelID, "RayE", sizeof(m_stSsPkg.m_stBody.m_stAccountLoginReq.m_szChannelID));
    StrCpy(m_stSsPkg.m_stBody.m_stAccountLoginReq.m_szAccountName, poPlayer->GetAccountName(), PKGMETA::MAX_NAME_LENGTH);
    ZoneSvrMsgLayer::Instance().SendToAccountSvr(m_stSsPkg);

    return 0;
}

int AccountLoginReq_CS::_HandleSDKLogin(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg)
{
    CS_PKG_ACCOUNT_LOGIN_REQ& rstAccountLoginReq = rstCsPkg.m_stBody.m_stAccountLoginReq;

    //同一账号重复登录，T下线
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (poPlayer)
    {
        m_stScPkg.m_stHead.m_wMsgId = SC_MSG_ACCOUNT_LOGIN_RSP;
        m_stScPkg.m_stBody.m_stAccountLoginRsp.m_nErrNo = ERR_ALREADY_EXISTED;
        ZoneSvrMsgLayer::Instance().SendToClient(pstSession, &m_stScPkg, PKGMETA::CONNSESSION_CMD_RELOGIN);
        LOGERR("Session(%u) relogin,kill off mac1<%s>, mac2<%s>", pstSession->m_dwSessionId, poPlayer->GetMacAddr(), rstAccountLoginReq.m_szMacAddress);

        return -1;
    }

    // 正常登陆流程
    poPlayer = PlayerMgr::Instance().NewPlayer();
    if (!poPlayer)
    {
        m_stScPkg.m_stHead.m_wMsgId = SC_MSG_ACCOUNT_LOGIN_RSP;
        m_stScPkg.m_stBody.m_stAccountLoginRsp.m_nErrNo = ERR_BAD_ALLOC;
        ZoneSvrMsgLayer::Instance().SendToClient(pstSession, &m_stScPkg, PKGMETA::CONNSESSION_CMD_STOP);
        LOGERR("Token(%s) create new player failed.", rstAccountLoginReq.m_szThirdSDKToken);
        return -2;
    }
    poPlayer->SetConnSession(pstSession);
    poPlayer->SetMacAddr(rstAccountLoginReq.m_szMacAddress);
    poPlayer->SetPhoneType(rstAccountLoginReq.m_szPhoneType);
    poPlayer->SetOSType(rstAccountLoginReq.m_szOSType);
    poPlayer->SetChannelName(rstAccountLoginReq.m_szChannelName);
    poPlayer->SetLoginType(rstAccountLoginReq.m_bLoginType);
	poPlayer->SetLoginSvrId(rstAccountLoginReq.m_dwLoginSvrId);
    poPlayer->SetSubChannelName(rstAccountLoginReq.m_szSubChannelName);
    PlayerMgr::Instance().AddToPlayerSessMap(poPlayer);

    // 设置客户端最高版本号
    poPlayer->SetProtocolVersion(rstAccountLoginReq.m_wVersion);

    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_SDK_ACCOUNT_LOGIN_REQ;
    m_stSsPkg.m_stHead.m_ullReservId = pstSession->m_dwSessionId;
    SS_PKG_SDK_ACCOUNT_LOGIN_REQ& rstSDKLoginReq = m_stSsPkg.m_stBody.m_stSDKAccountLoginReq;
    StrCpy(rstSDKLoginReq.m_szToken, rstAccountLoginReq.m_szThirdSDKToken, PKGMETA::MAX_LEN_SDK_TOKEN_PARA);
    StrCpy(rstSDKLoginReq.m_szChannelName, rstAccountLoginReq.m_szChannelName, PKGMETA::MAX_NAME_LENGTH);
    StrCpy(rstSDKLoginReq.m_szOpenID, rstAccountLoginReq.m_szOpenID, PKGMETA::MAX_NAME_LENGTH);

    ZoneSvrMsgLayer::Instance().SendToXiYouSDKSvr(m_stSsPkg);

    return 0;
}

int AccountLoginReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
    CS_PKG_ACCOUNT_LOGIN_REQ& rstAccountLoginReq = rstCsPkg.m_stBody.m_stAccountLoginReq;

    LOGRUN("handle account login req, Session(%u) AccountName(%s) PhoneType(%s) ChannelName(%s) LoginType(%d)",
            pstSession->m_dwSessionId, rstAccountLoginReq.m_szAccountName, rstAccountLoginReq.m_szPhoneType, rstAccountLoginReq.m_szChannelName, rstAccountLoginReq.m_bLoginType);

    int iSDKLoginSwitch = ZoneSvr::Instance().GetConfig().m_iLoginSDKSwitch;
	//rstAccountLoginReq.m_dwLoginSvrId = 1;
	//LOGWARN("LoginSvrId to del"); //临时加上 
    //不经过SDK的登录流程,内部测试和外部托管登录使用
    if (SDKLOGIN_SWITCH_OFF == iSDKLoginSwitch
	|| (SDKLOGIN_SWITCH_OFF != iSDKLoginSwitch && rstAccountLoginReq.m_bLoginType == ACCOUNT_TYPE_ESCROW)
	|| (rstAccountLoginReq.m_bLoginType == ACCOUNT_TYPE_GM))
    {
        //检查非法字符
        if (!_CheckNameVaild(rstAccountLoginReq.m_szAccountName))
        {
            m_stScPkg.m_stHead.m_wMsgId = SC_MSG_ACCOUNT_LOGIN_RSP;
            m_stScPkg.m_stBody.m_stAccountLoginRsp.m_nErrNo = ERR_INVALID_NAME;
            ZoneSvrMsgLayer::Instance().SendToClient(pstSession, &m_stScPkg, PKGMETA::CONNSESSION_CMD_STOP);
            return -1;
        }
        return _HandleConnect(pstSession, rstCsPkg);
    }
    //正式登录流程，需要先去SDK服务器通过token获取username
    else
    {
        return _HandleSDKLogin(pstSession, rstCsPkg);
    }

    return 0;
}


int ReconnectReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
    CS_PKG_RECONNECT_REQ& rstReconnReq = rstCsPkg.m_stBody.m_stReconnectReq;
    SC_PKG_RECONNECT_RSP& rstReconnRsp = m_stScPkg.m_stBody.m_stReconnectRsp;
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_RECONNECT_RSP;

    LOGRUN("handle Player(%lu) reconnect req", rstReconnReq.m_ullUin);

    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstReconnReq.m_ullUin);
    if (!poPlayer)
    {
        // 该玩家已经心跳超时，数据已经不存在于服务器，需要重新走登陆流程
        rstReconnRsp.m_nErrNo = ERR_RECONN_PLAYER_NOT_EXIST;
        ZoneSvrMsgLayer::Instance().SendToClient(pstSession, &m_stScPkg, PKGMETA::CONNSESSION_CMD_STOP);
        LOGERR("Reconn Player Uin(%lu) is not exist", rstReconnReq.m_ullUin);
        return -1;
    }

    if (!(poPlayer->GetState() == PLAYER_STATE_INGAME ||poPlayer->GetState() == PLAYER_STATE_RECONN_WAIT))
    {
        //玩家不在游戏中或等待重连状态
        rstReconnRsp.m_nErrNo = ERR_NOT_SATISFY_COND;
        ZoneSvrMsgLayer::Instance().SendToClient(pstSession, &m_stScPkg, PKGMETA::CONNSESSION_CMD_STOP);
        LOGERR("Reconn Player Uin(%lu) state(%d) is error", rstReconnReq.m_ullUin, poPlayer->GetState());
        return -1;
    }

    // 检查OnlineTokenId
    if (poPlayer->GetOnlineTokenId() != rstReconnReq.m_dwOnlineTokenId)
    {
        rstReconnRsp.m_nErrNo = ERR_RECONN_TOKENID;
        ZoneSvrMsgLayer::Instance().SendToClient(pstSession, &m_stScPkg, PKGMETA::CONNSESSION_CMD_STOP);
        LOGERR("Reconn Player(%s) Uin(%lu) online tokenid is error", poPlayer->GetRoleName(), poPlayer->GetUin());
        return -1;
    }

    uint32_t dwMinSeq;
    //检查SeqNo
    if (poPlayer->GetPkgSeqNo() != (rstReconnReq.m_dwSeqNum+1))
    {
        LOGRUN("Reconn Player(%s) Uin(%lu) Client Seq=(%u) Svr Seq=(%u)",
                poPlayer->GetRoleName(), poPlayer->GetUin(), rstReconnReq.m_dwSeqNum, poPlayer->GetPkgSeqNo() -1);

        rstReconnRsp.m_nErrNo = _CheckSeqNo(rstReconnReq.m_dwSeqNum, poPlayer, &dwMinSeq);
        if (rstReconnRsp.m_nErrNo != ERR_NONE)
        {
            ZoneSvrMsgLayer::Instance().SendToClient(pstSession, &m_stScPkg, PKGMETA::CONNSESSION_CMD_STOP);
            return -1;
        }
    }

    LOGRUN("Player(%s) Uin(%lu) Reconnet success", poPlayer->GetRoleName(), poPlayer->GetUin());

    // 重连成功，关闭旧的连接层的Conn
    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, NULL, PKGMETA::CONNSESSION_CMD_STOP);

    //更新session
    PlayerMgr::Instance().DelFromPlayerSessMap(poPlayer);
    poPlayer->SetConnSession(pstSession);
    PlayerMgr::Instance().AddToPlayerSessMap(poPlayer);

    poPlayer->GetConnSession()->m_chCmd = CONNSESSION_CMD_INPROC;
    rstReconnRsp.m_nErrNo = ERR_NONE;
    rstReconnRsp.m_dwSeqNum = poPlayer->m_dwRecvClientPkgSeq;

    ZoneSvrMsgLayer::Instance().SendToClient(pstSession, &m_stScPkg);

    if (poPlayer->GetPkgSeqNo() != (rstReconnReq.m_dwSeqNum+1))
    {
        _SendMissingPkg(rstReconnReq.m_dwSeqNum, poPlayer, dwMinSeq);
    }

    //重连成功，置玩家为InGame的状态
    PlayerStateMachine::Instance().ChangeState(poPlayer, PLAYER_STATE_INGAME);

    return 0;

}

int ReconnectReq_CS::_CheckSeqNo(uint32_t dwSeq, Player* poPlayer, uint32_t* pdwMinSeq)
{
    unsigned int dwLen = 0;
    size_t ulUseSize = 0;

    int iRet = poPlayer->m_oSendBuffer.PeekMsg(&m_szBuffer, &dwLen);
    if (iRet != THREAD_Q_SUCESS)
    {
        LOGERR("Reconn Player(%s) Uin(%lu) SendBuffer PeekMsg failed, Ret=(%d)", poPlayer->GetRoleName(), poPlayer->GetUin(), iRet);
        return ERR_SYS;
    }

    iRet = m_stPkgHead.unpack(m_szBuffer, dwLen, &ulUseSize, poPlayer->GetProtocolVersion());
    if (iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR("Reconn Player(%s) Uin(%lu) PkgHead unpack failed, Ret=(%d)", poPlayer->GetRoleName(), poPlayer->GetUin(), iRet);
        return ERR_SYS;
    }

    if (m_stPkgHead.m_dwSeqNum > dwSeq+1)
    {
        LOGERR("Reconn Player(%s) Uin(%lu) Svr Cache Seq=(%u), Client Seq=(%u)",
            poPlayer->GetRoleName(), poPlayer->GetUin(), m_stPkgHead.m_dwSeqNum, dwSeq);
        return ERR_NOT_SATISFY_COND;
    }

    *pdwMinSeq = m_stPkgHead.m_dwSeqNum;

    return ERR_NONE;
}

int ReconnectReq_CS::_SendMissingPkg(uint32_t dwSeq, Player* poPlayer, uint32_t dwMinSeq)
{
    unsigned int dwLen = 0;
    size_t ulUseSize = 0;
    uint32_t dwSvrSeq = poPlayer->GetPkgSeqNo() -1;
    int iCount = dwSvrSeq - dwMinSeq + 1;

    if (iCount < 0)
    {
        LOGERR("_SendMissingPkg iCount < 0");
        return ERR_SYS;
    }

    do
    {
        int iRet = poPlayer->m_oSendBuffer.PeekMsg(&m_szBuffer, &dwLen);
        if (iRet != THREAD_Q_SUCESS)
        {
            LOGERR("SendMissingPkg Player(%s) Uin(%lu) SendBuffer PeekMsg failed, Ret=(%d)",
                        poPlayer->GetRoleName(), poPlayer->GetUin(), iRet);
            return ERR_SYS;
        }

        iRet = m_stPkgHead.unpack(m_szBuffer, dwLen, &ulUseSize, poPlayer->GetProtocolVersion());
        if (iRet != TdrError::TDR_NO_ERROR)
        {
            LOGERR("SendMissingPkg Player(%s) Uin(%lu) PkgHead unpack failed, Ret=(%d)", poPlayer->GetRoleName(), poPlayer->GetUin(), iRet);
            return ERR_SYS;
        }

        poPlayer->m_oSendBuffer.PopMsg();

        if (m_stPkgHead.m_dwSeqNum > dwSeq)
        {
            LOGRUN("Player(%s) Uin(%lu) Reconn Sucess, Send Missing Pkg, MgsId=(%d), SeqNo=(%u)",
                    poPlayer->GetRoleName(), poPlayer->GetUin(), m_stPkgHead.m_wMsgId, m_stPkgHead.m_dwSeqNum);
            ZoneSvrMsgLayer::Instance().SendCachePkgToClient(poPlayer, m_szBuffer, dwLen);
        }

        iCount --;
    }while(m_stPkgHead.m_dwSeqNum < dwSvrSeq && iCount > 0);

    return ERR_NONE;
}

int AccountLoginRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    SS_PKG_ACCOUNT_LOGIN_RSP& rstSsAccLoginRsp = rstSsPkg.m_stBody.m_stAccountLoginRsp;
    SSDT_ACCOUNT_DATA& rstSsAccData = rstSsAccLoginRsp.m_stRspData.m_stAccountData;

    LOGRUN("receive account login rsp, Uin(%lu) Account(%s)", rstSsAccData.m_ullUin, rstSsAccLoginRsp.m_szAccountName);

    Player* poPlayer = PlayerMgr::Instance().GetPlayerByAccountName(rstSsAccLoginRsp.m_szAccountName);
    if (!poPlayer)
    {
        LOGERR("Uin<%lu>,Account<%s>: poPlayer is null", rstSsAccData.m_ullUin, rstSsAccLoginRsp.m_szAccountName);
        return ERR_DEFAULT;
    }

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_ACCOUNT_LOGIN_RSP;

    int iError = ERR_NONE;
    //RESBASIC* poBasic = NULL;
    do
    {
        if (rstSsAccLoginRsp.m_nErrNo != ERR_NONE)
        {
            LOGERR("Uin<%lu>,Account<%s>:account login failed, errno-%d", rstSsAccData.m_ullUin, rstSsAccLoginRsp.m_szAccountName, rstSsAccLoginRsp.m_nErrNo );
            iError = rstSsAccLoginRsp.m_nErrNo;
            break;
        }

        if (rstSsAccData.m_ullBanTime > (uint64_t)CGameTime::Instance().GetCurrSecond())
        {
            LOGERR("Uin<%lu>,Account<%s>: is banned!", rstSsAccData.m_ullUin, rstSsAccLoginRsp.m_szAccountName);
            iError = ERR_ACCOUNT_BAN;
            break;
        }

        uint8_t bLoginType = poPlayer->GetLoginType();
        if (SDKLOGIN_SWITCH_OFF != ZoneSvr::Instance().GetConfig().m_iLoginSDKSwitch)
        {
            //若为GM符合要求则直接跳过
            if (ACCOUNT_TYPE_GM == bLoginType)
            {
                if (ACCOUNT_TYPE_GM == rstSsAccData.m_bAccountType)
                {
                    break;
                }
                else
                {
                    iError = ERR_LOGIN_TYPE_ERROR;
                    LOGERR("Uin<%lu>, AccountName<%s>, AccountType<%hhu> LoginType<%hhu> AccountLogin error, Ret=(%d)",
                        rstSsAccData.m_ullUin, rstSsAccLoginRsp.m_szAccountName, rstSsAccData.m_bAccountType, bLoginType, iError);
                    break;
                }
            }

			//SDK部署时,需要校验 托管的登录方式 ,其他登录方式不用校验
            if (ACCOUNT_TYPE_ESCROW == bLoginType &&
                (rstSsAccData.m_bAccountType != ACCOUNT_TYPE_ESCROW && rstSsAccData.m_bAccountType != ACCOUNT_TYPE_TEST))
            {
                iError = ERR_LOGIN_TYPE_ERROR;
                LOGERR("Uin<%lu>, AccountName<%s>, AccountType<%hhu> LoginType<%hhu> AccountLogin error, Ret=(%d)",
                    rstSsAccData.m_ullUin, rstSsAccLoginRsp.m_szAccountName, rstSsAccData.m_bAccountType, bLoginType, iError);
                break;
            }

            if ((SDKLOGIN_SWITCH_TEST == ZoneSvr::Instance().GetConfig().m_iLoginSDKSwitch) &&
                (rstSsAccData.m_bAccountType != ACCOUNT_TYPE_TEST))
            {
                iError = ERR_SERVER_NOT_OPEN;
                time_t ulTmp = 0;
                CGameTime::Instance().ConvStrToTime(ZoneSvr::Instance().GetConfig().m_szSvrFinishUptTime, ulTmp, "%Y/%m/%d:%H:%M:%S");
                m_stScPkg.m_stBody.m_stAccountLoginRsp.m_ullSvrFinishUpdateTime = (uint64_t)ulTmp;
                LOGERR("Uin<%lu>, AccountName<%s>, AccountType<%hhu> LoginType<%hhu> AccountLogin error, Ret=(%d)",
                    rstSsAccData.m_ullUin, rstSsAccLoginRsp.m_szAccountName, rstSsAccData.m_bAccountType, bLoginType, iError);
                break;
            }

        }
    } while (false);

    m_stScPkg.m_stBody.m_stAccountLoginRsp.m_nErrNo = iError;

    if (iError != ERR_NONE)
    {
        PlayerLogic::Instance().PlayerLogout(poPlayer, PLAYER_LOGOUT_BY_LOGIC, &m_stScPkg);
        return iError;
    }

    poPlayer->SetUin(rstSsAccData.m_ullUin);
    poPlayer->SetAccountType(rstSsAccData.m_bAccountType);
    PlayerMgr::Instance().AddToPlayerUinMap(poPlayer);

    poPlayer->GetConnSession()->m_chCmd = CONNSESSION_CMD_INPROC;
    m_stScPkg.m_stBody.m_stAccountLoginRsp.m_ullUin = rstSsAccData.m_ullUin;
    m_stScPkg.m_stBody.m_stAccountLoginRsp.m_bAccountType = rstSsAccData.m_bAccountType;
    m_stScPkg.m_stBody.m_stAccountLoginRsp.m_dwOnlineTokenId = poPlayer->GetOnlineTokenId();
    m_stScPkg.m_stBody.m_stAccountLoginRsp.m_ullTimeStamp = (uint64_t)CGameTime::Instance().GetCurrSecond();
    m_stScPkg.m_stBody.m_stAccountLoginRsp.m_ullServerOpenTimeStamp = SvrTime::Instance().GetOpenSvrTime();
    StrCpy(m_stScPkg.m_stBody.m_stAccountLoginRsp.m_szAccountName, rstSsAccLoginRsp.m_szAccountName, PKGMETA::MAX_NAME_LENGTH);
    StrCpy(m_stScPkg.m_stBody.m_stAccountLoginRsp.m_stSeverConfig.m_szUrl, ZoneSvr::Instance().GetConfig().m_szFeedBackUrl, MAX_LEN_URL);
    m_stScPkg.m_stBody.m_stAccountLoginRsp.m_stSeverConfig.m_chSerialNumSwitch = ZoneSvr::Instance().GetConfig().m_iSerialNumSwitch;
    m_stScPkg.m_stBody.m_stAccountLoginRsp.m_stSeverConfig.m_chTestPaySwitch = ZoneSvr::Instance().GetConfig().m_iTestPaySwitch;

	//功能开关
	m_stScPkg.m_stBody.m_stAccountLoginRsp.m_stSeverConfig.m_llCommonSwitch = 
		(int64_t)ZoneSvr::Instance().GetConfig().m_iGuildFightSwitch << COMMON_SWITCH_TYPE_GUILD_FIGHT
		| (int64_t)ZoneSvr::Instance().GetConfig().m_iMineSwitch << COMMON_SWITCH_TYPE_MINE;
    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

int SDKAccountLoginRsp_SS::HandleServerMsg(SSPKG& rstSsPkg, void* pvPara)
{
    uint32_t dwSessionId = rstSsPkg.m_stHead.m_ullReservId;

    LOGRUN("handle SdkAccount login Rsp, Session Id(%u)", dwSessionId);

    SS_PKG_SDK_ACCOUNT_LOGIN_RSP& rstSsSDKLoginRsp = rstSsPkg.m_stBody.m_stSDKAccountLoginRsp;

    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(dwSessionId);
    if (!poPlayer)
    {
        LOGERR("poPlayer is not found by Session Id(%u)", dwSessionId);
        return ERR_DEFAULT;
    }

    //SDK服务器验证Token失败
    if (ERR_NONE != rstSsSDKLoginRsp.m_nErrNo)
    {
        m_stScPkg.m_stHead.m_wMsgId = SC_MSG_ACCOUNT_LOGIN_RSP;
        m_stScPkg.m_stBody.m_stAccountLoginRsp.m_nErrNo = rstSsSDKLoginRsp.m_nErrNo;
        ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg, PKGMETA::CONNSESSION_CMD_STOP);
        PlayerMgr::Instance().DeletePlayer(poPlayer);
        LOGERR("account SDK login failed, Session Id(%u)", dwSessionId);
        return ERR_DEFAULT;
    }

    //玩家是否已存在s
    Player* poOldPlayer = PlayerMgr::Instance().GetPlayerBySdkUserNameAndSvrId(rstSsSDKLoginRsp.m_szUid, poPlayer->GetLoginSvrId());
    if (poOldPlayer)
    {
        _HandleReconn(poOldPlayer, poPlayer);
    }
    else
    {
        _HandleSdkLogin(rstSsPkg, poPlayer);
    }

    return 1;
}

int SDKAccountLoginRsp_SS::_HandleReconn(Player* poOldPlayer, Player* poNewPlayer)
{
    assert(poOldPlayer && poNewPlayer);

    SC_PKG_ACCOUNT_LOGIN_RSP& rstAccountLoginRsp = m_stScPkg.m_stBody.m_stAccountLoginRsp;
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_ACCOUNT_LOGIN_RSP;

    if (poOldPlayer->IsCurState(PLAYER_STATE_RECONN_WAIT) ||
       (poOldPlayer->IsCurState(PLAYER_STATE_INGAME) && strcmp(poOldPlayer->GetMacAddr(), poNewPlayer->GetMacAddr()) == 0))
    {
        LOGRUN("Player(%s) Uin(%lu) Account(%s) Login, but in Reconn_Wait state, Reconn",
                 poOldPlayer->GetRoleName(), poOldPlayer->GetUin(), poOldPlayer->GetAccountName());

        //更新session
        PlayerMgr::Instance().DelFromPlayerSessMap(poOldPlayer);
        poOldPlayer->SetConnSession(poNewPlayer->GetConnSession());
        poOldPlayer->GetConnSession()->m_chCmd = CONNSESSION_CMD_INPROC;
        poOldPlayer->SetProtocolVersion(poNewPlayer->GetProtocolVersion());
        PlayerMgr::Instance().DeletePlayer(poNewPlayer);
        PlayerMgr::Instance().AddToPlayerSessMap(poOldPlayer);

        // 更新在线token,用于断线重连
        poOldPlayer->SetOnlineTokenId(CFakeRandom::Instance().Random());
        poOldPlayer->SetLastLoginTime(CGameTime::Instance().GetCurrSecond());

        rstAccountLoginRsp.m_nErrNo = ERR_NONE;
        rstAccountLoginRsp.m_ullUin = poOldPlayer->GetUin();
        rstAccountLoginRsp.m_dwOnlineTokenId = poOldPlayer->GetOnlineTokenId();
        rstAccountLoginRsp.m_ullTimeStamp = (uint64_t)CGameTime::Instance().GetCurrSecond();
        StrCpy(rstAccountLoginRsp.m_szAccountName, poOldPlayer->GetAccountName(), PKGMETA::MAX_NAME_LENGTH);

        //开服时间
        rstAccountLoginRsp.m_ullServerOpenTimeStamp = SvrTime::Instance().GetOpenSvrTime();
        StrCpy(rstAccountLoginRsp.m_stSeverConfig.m_szUrl, ZoneSvr::Instance().GetConfig().m_szFeedBackUrl, MAX_LEN_URL);
        rstAccountLoginRsp.m_stSeverConfig.m_chSerialNumSwitch = ZoneSvr::Instance().GetConfig().m_iSerialNumSwitch;
        m_stScPkg.m_stBody.m_stAccountLoginRsp.m_stSeverConfig.m_chTestPaySwitch = ZoneSvr::Instance().GetConfig().m_iTestPaySwitch;
		//功能开关
		m_stScPkg.m_stBody.m_stAccountLoginRsp.m_stSeverConfig.m_llCommonSwitch =
			(int64_t)ZoneSvr::Instance().GetConfig().m_iGuildFightSwitch << COMMON_SWITCH_TYPE_GUILD_FIGHT
			| (int64_t)ZoneSvr::Instance().GetConfig().m_iMineSwitch << COMMON_SWITCH_TYPE_MINE;
        ZoneSvrMsgLayer::Instance().SendToClient(poOldPlayer, &m_stScPkg);

        //切换为重连登录状态
        PlayerStateMachine::Instance().ChangeState(poOldPlayer, PLAYER_STATE_RECONN_LOGIN);
    }
    else
    {
        m_stScPkg.m_stBody.m_stAccountLoginRsp.m_nErrNo = ERR_ALREADY_EXISTED;
        ZoneSvrMsgLayer::Instance().SendToClient(poNewPlayer, &m_stScPkg, PKGMETA::CONNSESSION_CMD_STOP);
        PlayerMgr::Instance().DeletePlayer(poNewPlayer);

        LOGERR("Player(%s) Uin(%lu) Account(%s) login failed, because this player is already existed.",
                poOldPlayer->GetRoleName(), poOldPlayer->GetUin(), poOldPlayer->GetAccountName());
    }

    return 1;
}

int SDKAccountLoginRsp_SS::_HandleSdkLogin(PKGMETA::SSPKG& rstSsPkg, Player* poPlayer)
{
    SS_PKG_SDK_ACCOUNT_LOGIN_RSP& rstSsSDKLoginRsp = rstSsPkg.m_stBody.m_stSDKAccountLoginRsp;

    LOGRUN("handle sdk Login, Account(%s), Session(%lu)", rstSsSDKLoginRsp.m_szUid, rstSsPkg.m_stHead.m_ullReservId);

    poPlayer->SetAccountName(rstSsSDKLoginRsp.m_szUid);
	poPlayer->SetSdkUserName(rstSsSDKLoginRsp.m_szUid);
    PlayerMgr::Instance().AddToAccountNameHM(poPlayer);

    // 设置随机在线token,用于断线重连
    poPlayer->SetOnlineTokenId(CFakeRandom::Instance().Random());
    poPlayer->SetLastUptDbTime(CGameTime::Instance().GetCurrSecond());
    poPlayer->SetLastLoginTime(CGameTime::Instance().GetCurrSecond());

    PlayerStateMachine::Instance().ChangeState(poPlayer, PLAYER_STATE_LOGIN);

    //发给AccountSvr处理
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_ACCOUNT_LOGIN_REQ;

    SS_PKG_ACCOUNT_LOGIN_REQ& rstAccountLoginReq = m_stSsPkg.m_stBody.m_stAccountLoginReq;
    StrCpy(rstAccountLoginReq.m_szAccountName, poPlayer->GetAccountName(), PKGMETA::MAX_NAME_LENGTH);	//用游戏内合成的AccountName
    StrCpy(rstAccountLoginReq.m_szChannelID, rstSsSDKLoginRsp.m_szExt, PKGMETA::MAX_LEN_SDK_PARA);
	rstAccountLoginReq.m_dwLoginSvrId = poPlayer->GetLoginSvrId();
    ZoneSvrMsgLayer::Instance().SendToAccountSvr(m_stSsPkg);

    return ERR_NONE;
}

int RoleCreateRsp_SS::HandleServerMsg(SSPKG& rstSsRspPkg, void* pvPara)
{
    LOGRUN("receive role login server msg");

    SS_PKG_ROLE_CREATE_RSP& rstSsRoleCreateRsp = rstSsRspPkg.m_stBody.m_stRoleCreateRsp;
    CGameTime& roGameTime = CGameTime::Instance();

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_ROLE_LOGIN_RSP;

    COMMON_PKG_ROLE_LOGIN_RSP& rstRoleLoginRsp = m_stScPkg.m_stBody.m_stRoleLoginRsp;
    rstRoleLoginRsp.m_ullUin = rstSsRoleCreateRsp.m_ullUin;
    bzero(&rstRoleLoginRsp.m_stServerTime, sizeof(rstRoleLoginRsp.m_stServerTime));

    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsRoleCreateRsp.m_ullUin);

    if (poPlayer)
    {
        if (rstSsRoleCreateRsp.m_nErrNo != ERR_NONE)
        {
            rstRoleLoginRsp.m_chResult = 0;
            rstRoleLoginRsp.m_nErrNo = rstSsRoleCreateRsp.m_nErrNo;
            rstRoleLoginRsp.m_ullUin = rstSsRoleCreateRsp.m_ullUin;
            PlayerLogic::Instance().PlayerLogout(poPlayer, PLAYER_LOGOUT_BY_LOGIC, &m_stScPkg);
            LOGERR("Uin<%lu> role create failed, errno<%d>", rstSsRoleCreateRsp.m_ullUin, rstSsRoleCreateRsp.m_nErrNo);
            return -1;
        }

        // DB新建角色成功
        rstRoleLoginRsp.m_ullUin = poPlayer->GetUin();
        if (!poPlayer->GetPlayerData().PackRoleWholeData(rstRoleLoginRsp.m_stRspData.m_stRoleWholeData))
        {
            rstRoleLoginRsp.m_chResult = 0;
            rstRoleLoginRsp.m_nErrNo = ERR_SYS;
            PlayerLogic::Instance().PlayerLogout(poPlayer, PLAYER_LOGOUT_BY_LOGIC, &m_stScPkg);
            LOGERR("Uin<%lu> pack role data failed", rstRoleLoginRsp.m_ullUin);
            return -1;
        }

        rstRoleLoginRsp.m_chResult = 1;
        rstRoleLoginRsp.m_nErrNo = ERR_NONE;
        timeval* pstTime = roGameTime.GetCurrTime();
        rstRoleLoginRsp.m_stServerTime.m_llSec = pstTime->tv_sec;
        rstRoleLoginRsp.m_stServerTime.m_nMSec = pstTime->tv_usec/1000;
        ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

        PlayerStateMachine::Instance().ChangeState(poPlayer, PLAYER_STATE_INGAME);
        PlayerMgr::Instance().AddToRoleNameHM(poPlayer);
        poPlayer->SetLastLoginTime(roGameTime.GetCurrSecond());

        Tutorial::Instance()._SendTutorialGift(&poPlayer->GetPlayerData());

        // 发送全局账号表记录
        bzero(&m_stSsPkg.m_stHead, sizeof(m_stSsPkg.m_stHead));
        m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_CLUSTER_ACC_NEW_ROLE_REG;
        m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();
        m_stSsPkg.m_stBody.m_stClusterAccNewRoleReg.m_iServerID = ZoneSvr::Instance().GetConfig().m_wSvrId;
        m_stSsPkg.m_stBody.m_stClusterAccNewRoleReg.m_ullUin = poPlayer->GetUin();
        StrCpy( m_stSsPkg.m_stBody.m_stClusterAccNewRoleReg.m_szSdkUserName, poPlayer->GetSdkUserName(), PKGMETA::MAX_NAME_LENGTH );
        StrCpy( m_stSsPkg.m_stBody.m_stClusterAccNewRoleReg.m_szRoleName, poPlayer->GetRoleName(), PKGMETA::MAX_NAME_LENGTH );
        StrCpy( m_stSsPkg.m_stBody.m_stClusterAccNewRoleReg.m_szChannelID, poPlayer->GetSubChannelName(), PKGMETA::MAX_NAME_LENGTH );
        ZoneSvrMsgLayer::Instance().SendToClusterAccSvr( m_stSsPkg );

        LOGRUN("role create success, uin<%lu>, account<%s>", poPlayer->GetUin(), poPlayer->GetAccountName());
    }

    return 0;
}

int RoleLoginReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
    Player* poPlayer = (Player*)pvPara;
    if (!poPlayer)
    {
        LOGERR("poPlayer is null");
        return -1;
    }

    LOGRUN("receive role login client request, Player(%s) Uin(%lu) Account(%s)",
             poPlayer->GetRoleName(), poPlayer->GetUin(), poPlayer->GetAccountName());

    if (!poPlayer->IsCurState(PLAYER_STATE_LOGIN) && !poPlayer->IsCurState(PLAYER_STATE_RECONN_LOGIN))
    {
        LOGERR("player is not in login state, kill off! account name: (%s)", poPlayer->GetAccountName());
        PlayerLogic::Instance().PlayerLogout(poPlayer, PLAYER_LOGOUT_BY_LOGIC);
        return -1;
    }

    if (poPlayer->IsCurState(PLAYER_STATE_RECONN_LOGIN))
    {
        // 重连不从数据库取数据
        m_stScPkg.m_stHead.m_wMsgId = SC_MSG_ROLE_LOGIN_RSP;
        COMMON_PKG_ROLE_LOGIN_RSP& rstRoleLoginRsp = m_stScPkg.m_stBody.m_stRoleLoginRsp;
        rstRoleLoginRsp.m_ullUin = poPlayer->GetUin();
        if (!poPlayer->GetPlayerData().PackRoleWholeData(rstRoleLoginRsp.m_stRspData.m_stRoleWholeData))
        {
            rstRoleLoginRsp.m_chResult = 0;
            rstRoleLoginRsp.m_nErrNo = ERR_SYS;
            PlayerLogic::Instance().PlayerLogout(poPlayer, PLAYER_LOGOUT_BY_LOGIC, &m_stScPkg);
            LOGERR("pack role data failed");
            return -1;
        }

        rstRoleLoginRsp.m_chResult = 1;
        rstRoleLoginRsp.m_nErrNo = ERR_NONE;
        timeval* pstTime = CGameTime::Instance().GetCurrTime();
        rstRoleLoginRsp.m_stServerTime.m_llSec = pstTime->tv_sec;
        rstRoleLoginRsp.m_stServerTime.m_nMSec = pstTime->tv_usec/1000;
        ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

        PlayerStateMachine::Instance().ChangeState(poPlayer, PLAYER_STATE_INGAME);

        poPlayer->AfterRoleLogin();

        return 0;
    }

    // 从数据库取数据
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_ROLE_LOGIN_REQ;
    m_stSsPkg.m_stHead.m_iDstProcId = ZoneSvr::Instance().GetConfig().m_iRoleSvrID;
    m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();
    m_stSsPkg.m_stBody.m_stRoleLoginReq = rstCsPkg.m_stBody.m_stRoleLoginReq;
    ZoneSvrMsgLayer::Instance().SendToRoleSvr(m_stSsPkg);

    return 0;
}

int RoleLoginRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    COMMON_PKG_ROLE_LOGIN_RSP& rstSSRoleLoginRsp = rstSsPkg.m_stBody.m_stRoleLoginRsp;
    LOGRUN("RoleLoginRsp_SS:Uin(%lu) receive role login server msg", rstSSRoleLoginRsp.m_ullUin);

    ZoneSvrMsgLayer& roZoneSvrMsgLayer = ZoneSvrMsgLayer::Instance();

    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSSRoleLoginRsp.m_ullUin);
    if (!poPlayer)
    {
        LOGERR("can not find player, uin-<%lu>", rstSSRoleLoginRsp.m_ullUin);
        return -1;
    }

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_ROLE_LOGIN_RSP;
    COMMON_PKG_ROLE_LOGIN_RSP& rstRoleLoginRsp = m_stScPkg.m_stBody.m_stRoleLoginRsp;
    bzero(&rstRoleLoginRsp.m_stServerTime, sizeof(rstRoleLoginRsp.m_stServerTime));

    if (ERR_NONE != rstSSRoleLoginRsp.m_nErrNo)
    {
        if (ERR_NOT_FOUND == rstSSRoleLoginRsp.m_nErrNo)
        {
            // 创建新玩家
            poPlayer->InitNewPlayer();

            //创建新玩家日志
            ZoneLog::Instance().WriteCreateNewLog(poPlayer);

            // save database
            m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_ROLE_CREATE_REQ;
            m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();
            SS_PKG_ROLE_CREATE_REQ& rstRoleCreateReq = m_stSsPkg.m_stBody.m_stRoleCreateReq;
            if (!poPlayer->GetPlayerData().PackRoleWholeData(rstRoleCreateReq.m_stRoleWholeData))
            {
                rstRoleLoginRsp.m_chResult = 0;
                rstRoleLoginRsp.m_nErrNo = ERR_SYS;
                PlayerLogic::Instance().PlayerLogout(poPlayer, PLAYER_LOGOUT_BY_LOGIC, &m_stScPkg);
                LOGERR("create new role account-%s, pack whole role data failed!", poPlayer->GetAccountName());
                return -1;
            }

            ZoneSvrMsgLayer::Instance().SendToRoleSvr(m_stSsPkg);

            // 通知创建邮箱
            Mail::Instance().SendPlayerStatToMailSvr(poPlayer->GetUin(), PLAYER_STATE_CREATE);

            LOGRUN("RoleLoginRsp_SS:Account<%s> Craete new role account!", poPlayer->GetAccountName());

            return 0;
        }

        rstRoleLoginRsp.m_chResult = 0;
        rstRoleLoginRsp.m_nErrNo = rstSSRoleLoginRsp.m_nErrNo;
        PlayerLogic::Instance().PlayerLogout(poPlayer, PLAYER_LOGOUT_BY_LOGIC, &m_stScPkg);
    }
    else
    {
        // 已存在的玩家
        if (!poPlayer->InitFromDB(rstSSRoleLoginRsp.m_stRspData.m_stRoleWholeData))
        {
            // 从数据库初始化玩家出错
            rstRoleLoginRsp.m_chResult = 0;
            rstRoleLoginRsp.m_nErrNo = ERR_SYS;
            bzero(&rstRoleLoginRsp.m_stServerTime, sizeof(rstRoleLoginRsp.m_stServerTime));
            PlayerLogic::Instance().PlayerLogout(poPlayer, PLAYER_LOGOUT_BY_LOGIC, &m_stScPkg);
            LOGERR("init role data from database failed, account %s", poPlayer->GetAccountName());
            return -1;
        }

        rstRoleLoginRsp.m_ullUin = poPlayer->GetUin();
        if (!poPlayer->GetPlayerData().PackRoleWholeData(rstRoleLoginRsp.m_stRspData.m_stRoleWholeData))
        {
            rstRoleLoginRsp.m_chResult = 0;
            rstRoleLoginRsp.m_nErrNo = ERR_SYS;
            PlayerLogic::Instance().PlayerLogout(poPlayer, PLAYER_LOGOUT_BY_LOGIC, &m_stScPkg);
            LOGERR("role login pack data failed, account<%s>, Uin<%lu>, RoleName<%s>", poPlayer->GetAccountName(), rstRoleLoginRsp.m_ullUin,
                rstSSRoleLoginRsp.m_stRspData.m_stRoleWholeData.m_stBaseInfo.m_szRoleName);
            return -1;
        }

        rstRoleLoginRsp.m_chResult = 1;
        rstRoleLoginRsp.m_nErrNo = ERR_NONE;
        rstRoleLoginRsp.m_stServerTime.m_llSec = CGameTime::Instance().GetCurrTime()->tv_sec;
        rstRoleLoginRsp.m_stServerTime.m_nMSec = CGameTime::Instance().GetCurrTime()->tv_usec / 1000;
        roZoneSvrMsgLayer.SendToClient(poPlayer, &m_stScPkg);

        LOGRUN("Player(%s) Role login, Uin(%lu), Account(%s)",
                poPlayer->GetRoleName(), poPlayer->GetUin(), poPlayer->GetAccountName());

        //记录玩家登陆日志
        ZoneLog::Instance().WriteLoginLog(poPlayer);

        //登录后处理
        poPlayer->AfterRoleLogin();
    }

    return 0;
}

int AccountLogoutReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
    LOGRUN("receive account logout client request");

    Player* poPlayer = (Player*)pvPara;
    if (!poPlayer)
    {
        LOGERR("poPlayer is null");
        return -1;
    }

    PlayerLogic::Instance().PlayerLogout(poPlayer, PLAYER_LOGOUT_BY_LOGIC);

    return 0;
}

int MajestyRegReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
    CS_PKG_MAJESTY_REGISTER_REQ& rstCsMajestyRegReq = rstCsPkg.m_stBody.m_stMajestyRegisterReq;

    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_MAJESTY_REGISTER_REQ;

    SS_PKG_MAJESTY_REGISTER_REQ& rstSsMajestyRegReq= m_stSsPkg.m_stBody.m_stMajestyRegReq;

    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstCsMajestyRegReq.m_ullUin);

    if (!poPlayer)
    {
        LOGERR("poPlayer is not found, Uin(%lu)", rstCsMajestyRegReq.m_ullUin);
        return -1;
    }


    //已有名字了,不能调用
    if ( poPlayer->GetRoleName()[0] != '\0' )
    {

        SC_PKG_MAJESTY_REGISTER_RSP& rstScMajestyRegRsp = m_stScPkg.m_stBody.m_stMajestyRegisterRsp;
        m_stScPkg.m_stHead.m_wMsgId = SC_MSG_MAJESTY_REGISTER_RSP;
        rstScMajestyRegRsp.m_nErrNo = ERR_SYS;
        ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
        LOGERR("Uin<%lu> MajestyReg error, have  role name <%s>, ", rstCsMajestyRegReq.m_ullUin, poPlayer->GetRoleName());
        return -1;
    }

    //检查是否含有非法字符,如果含有则考虑为外挂
    if (IsContainSpecialChar(rstSsMajestyRegReq.m_szName))
    {
        m_stScPkg.m_stHead.m_wMsgId = SC_MSG_MAJESTY_REGISTER_RSP;
        m_stScPkg.m_stBody.m_stAccountLoginRsp.m_nErrNo = ERR_INVALID_NAME;
        ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

        LOGERR("Invaild RoleName, Uin(%lu)", rstCsMajestyRegReq.m_ullUin);
        return -1;
    }

    rstSsMajestyRegReq.m_ullUin = rstCsMajestyRegReq.m_ullUin;
    StrCpy(rstSsMajestyRegReq.m_szName, rstCsMajestyRegReq.m_szName, PKGMETA::MAX_NAME_LENGTH);

    ZoneSvrMsgLayer::Instance().SendToRoleSvr(m_stSsPkg);

    return 0;
}

int MajestyRegRsp_SS::HandleServerMsg(SSPKG& rstSsRspPkg, void* pvPara)
{
    SS_PKG_MAJESTY_REGISTER_RSP& rstSsMajestyRegRsp = rstSsRspPkg.m_stBody.m_stMajestyRegRsp;

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_MAJESTY_REGISTER_RSP;
    SC_PKG_MAJESTY_REGISTER_RSP& rstScMajestyRegRsp= m_stScPkg.m_stBody.m_stMajestyRegisterRsp;

    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsMajestyRegRsp.m_ullUin);
    if (!poPlayer)
    {
        LOGERR("poPlayer is not found, Uin(%lu)", rstSsMajestyRegRsp.m_ullUin);
        return -1;
    }

    rstScMajestyRegRsp.m_nErrNo = rstSsMajestyRegRsp.m_nErrNo;
    if (rstSsMajestyRegRsp.m_nErrNo != ERR_NONE)
    {
        ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

        return -1;
    }

    poPlayer->SetRoleName(rstSsMajestyRegRsp.m_szName);
    rstScMajestyRegRsp.m_ullUin = rstSsMajestyRegRsp.m_ullUin;
    StrCpy(rstScMajestyRegRsp.m_szName, rstSsMajestyRegRsp.m_szName, PKGMETA::MAX_NAME_LENGTH);

    // 更新全局帐号表名字
    bzero(&m_stSsPkg.m_stHead, sizeof(m_stSsPkg.m_stHead));
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_CLUSTER_ACC_CHG_ROLE_NAME;
    m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();
    m_stSsPkg.m_stBody.m_stClusterAccChgRoleName.m_ullUin = poPlayer->GetUin();
    StrCpy( m_stSsPkg.m_stBody.m_stClusterAccChgRoleName.m_szSdkUserName, poPlayer->GetSdkUserName(), PKGMETA::MAX_NAME_LENGTH );
    StrCpy( m_stSsPkg.m_stBody.m_stClusterAccChgRoleName.m_szNewRoleName, poPlayer->GetRoleName(), PKGMETA::MAX_NAME_LENGTH );
    ZoneSvrMsgLayer::Instance().SendToClusterAccSvr( m_stSsPkg );


    //记录玩家首次登陆日志，记录在这里的原因是玩家首次登陆在这里才创建角色名，如果记录在rolelogin ss消息处理的地方无法记录首次进入游戏时的角色名
    ZoneLog::Instance().WriteLoginLog(poPlayer);

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 1;
}
