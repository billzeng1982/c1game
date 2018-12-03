#include "LogMacros.h"
#include "ObjectPool.h"
#include "FightSvrMsgLayer.h"
#include "../FightSvr.h"
#include "../player/PlayerMgr.h"
#include "../player/PlayerLogic.h"
#include "../logic/MsgLogicDungeon.h"
#include "../logic/MsgLogicLogin.h"
#include "GameTime.h"
#include "CpuSampleStats.h"
#include "define.h"

using namespace PKGMETA;

FightSvrMsgLayer::FightSvrMsgLayer() : CMsgLayerBase(sizeof(PKGMETA::SCPKG) + 256, sizeof(PKGMETA::SSPKG) + 256)
{
	m_iConnID = 0;
}

void FightSvrMsgLayer::_RegistClientMsg() {
    REGISTER_MSG_HANDLER(CS_MSG_FIGHT_LOGIN_REQ,				FightLogin_CS,				m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_FIGHT_LOGOUT,					FightLogout_CS,				m_oCsMsgHandlerMap);
	REGISTER_MSG_HANDLER(CS_MSG_FIGHT_LOADING_PROGRESS_NTY,		FightLoadingProgress_CS,	m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_CHOOSE_GENERAL_REQ,				ChooseGeneralReq_CS,	    m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_FIGHT_CHG_SKIN_REQ,				FightChgSkinReq_CS,	        m_oCsMsgHandlerMap);
	REGISTER_MSG_HANDLER(CS_MSG_DUNGEON_START_REQ,				DungeonStartReq_CS,			m_oCsMsgHandlerMap);
	REGISTER_MSG_HANDLER(CS_MSG_DUNGEON_OP_END_NTF,				DungeonOpEndNtf_CS,			m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_FIGHT_POS_NTF,					FightPosNtf_CS,				m_oCsMsgHandlerMap);
	REGISTER_MSG_HANDLER(CS_MSG_FIGHT_SPEED_NTF,				FightSpeedNtf_CS,			m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_FIGHT_HP_NTF,					FightHpNtf_CS,				m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_FIGHT_MORALE_NTF,				FightMoraleNtf_CS,			m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_FIGHT_ATK_LOCK_NTF,				FightAtkLockNtf_CS,			m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_FIGHT_SKILL_START_NTF,			FightSkillStartNtf_CS,		m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_FIGHT_SKILL_FINISH_NTF,			FightSkillFinishNtf_CS,		m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_FIGHT_BUFF_ADD_NTF,				FightBuffAddNtf_CS,			m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_FIGHT_BUFF_DEL_NTF,				FightBuffDelNtf_CS,			m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_FIGHT_SOLO_START_NTF,			FightSoloStartNtf_CS,		m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_FIGHT_SOLO_SETTLE_NTF,			FightSoloSettleNtf_CS,		m_oCsMsgHandlerMap);
	REGISTER_MSG_HANDLER(CS_MSG_FIGHT_SOLO_FINISH_NTF,			FightSoloFinishNtf_CS,		m_oCsMsgHandlerMap);
	REGISTER_MSG_HANDLER(CS_MSG_FIGHT_MSKILL_START_NTF,			FightMSkillStartNtf_CS,		m_oCsMsgHandlerMap);
	REGISTER_MSG_HANDLER(CS_MSG_FIGHT_MSKILL_FINISH_NTF,		FightMSkillFinishNtf_CS,	m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_FIGHT_RETREATING_NTF,			FightRetreatingNtf_CS,		m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_FIGHT_RETREAT_NTF,				FightRetreatNtf_CS,			m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_FIGHT_OUTCITY_NTF,				FightOutCityNtf_CS,			m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_FIGHT_DEAD_PLAY_END_NTF,		FightDeadPlayEndNtf_CS,		m_oCsMsgHandlerMap);
	REGISTER_MSG_HANDLER(CS_MSG_FIGHT_AMBUSH_START_NTF,			FightAmbushStartNtf_CS,		m_oCsMsgHandlerMap);
	REGISTER_MSG_HANDLER(CS_MSG_FIGHT_AMBUSH_FINISH_NTF,		FightAmbushFinishNtf_CS,	m_oCsMsgHandlerMap);
	REGISTER_MSG_HANDLER(CS_MSG_FIGHT_ATKCITY_NTF,				FightAtkCityNtf_CS,			m_oCsMsgHandlerMap);
	REGISTER_MSG_HANDLER(CS_MSG_FIGHT_CATAPULT_INFO_NTF,		FightCatapultInfoNtf_CS,	m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_TIME_SYNC_REQ,					FightTimeSyncReq_CS,		m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_TEST_PACKAGE_INGRESS,			TestPackgeIngress_CS,		m_oCsMsgHandlerMap);
	REGISTER_MSG_HANDLER(CS_MSG_TIME_DELAY_REQ,					FightDelaySyncReq_CS,		m_oCsMsgHandlerMap);
    REGISTER_MSG_HANDLER(CS_MSG_FIGHT_SURRENDER_NTF,		    FightSurrenderReq_CS,		m_oCsMsgHandlerMap);
	REGISTER_MSG_HANDLER(CS_MSG_FIGHT_STATE_NTF,				FightStateNtf_CS,			m_oCsMsgHandlerMap);
}

void FightSvrMsgLayer::_RegistServerMsg()
{
    REGISTER_MSG_HANDLER(SS_MSG_DUNGEON_CREATE_REQ,             DungeonCreateReq_SS,        m_oSsMsgHandlerMap);
}

bool FightSvrMsgLayer::Init()
{
	FIGHTSVRCFG& rConfig = CFightSvr::Instance().GetConfig();
	if (!this->_Init(rConfig.m_iBusGCIMKey, rConfig.m_iProcID))
    {
		return false;
	}
	m_iConnID = rConfig.m_iConnID;

	return true;
}

int FightSvrMsgLayer::DealPkg()
{
	FIGHTSVRCFG& rConfig = CFightSvr::Instance().GetConfig();

	int iSrc = 0;
	int iRecvBytes = 0;
	int i = 0;

	for (; i < DEAL_PKG_PER_LOOP; i++)
    {
		iRecvBytes = m_oCommBusLayer.Recv();
		if (iRecvBytes < 0)
        {
			LOGERR("bus recv error!");
			return -1;
		}

		if (0 == iRecvBytes)
        {
			break;
		}

		iSrc = m_oCommBusLayer.GetRecvMsgSrc();
		if (rConfig.m_iConnID == iSrc)
        {
			this->_DealConnPkg();
		}
        else
        {
			this->_DealSvrPkg();
		}
	}

	return i;
}


#define MAX_DELTA_TIME 500
void FightSvrMsgLayer::_DealConnPkg()
{
	MyTdrBuf* pstTdrBuf = m_oCommBusLayer.GetRecvTdrBuf();

	PKGMETA::CONNSESSION stSession;
	size_t uUsedLen = 0;
	Player* poPlayer = NULL;

	TdrError::ErrorType iRet = stSession.unpack(pstTdrBuf->m_szTdrBuf, pstTdrBuf->m_uPackLen, &uUsedLen);
	if (iRet != TdrError::TDR_NO_ERROR)
    {
		LOGERR("Unpack ConnSession failed!");
		return;
	}

	if (PKGMETA::CONNSESSION_CMD_STOP == stSession.m_chCmd)
    {
		// 断线
		poPlayer = PlayerMgr::Instance().GetBySessionId(stSession.m_dwSessionId);
		if (poPlayer)
        {
			PlayerLogic::Instance().OnSessionStop(poPlayer, stSession.m_stCmdData.m_stConnSessStop.m_chStopReason);
		}
	}

	if (pstTdrBuf->m_uPackLen > uUsedLen)
    {
		char* pszPkg = pstTdrBuf->m_szTdrBuf + uUsedLen;
		TdrError::ErrorType iRet = m_stCsRecvPkg.unpack((const char*) pszPkg, pstTdrBuf->m_uPackLen - uUsedLen);
		if (iRet != TdrError::TDR_NO_ERROR)
        {
			return;
		}

		IMsgBase* poMsgHandler = this->GetClientMsgHandler(m_stCsRecvPkg.m_stHead.m_wMsgId);
		if (!poMsgHandler)
        {
			LOGERR("Can not find msg handler. id <%u>", m_stCsRecvPkg.m_stHead.m_wMsgId);
			return;
		}

        //检查时间戳
        if (m_stCsRecvPkg.m_stHead.m_ullTimeStamp != 0)
        {
            uint64_t ullCurTime = CGameTime::Instance().GetCurrTimeMs();
            if ((m_stCsRecvPkg.m_stHead.m_ullTimeStamp > ullCurTime) &&
              (m_stCsRecvPkg.m_stHead.m_ullTimeStamp - ullCurTime) >= MAX_DELTA_TIME)
            {
                poPlayer = PlayerMgr::Instance().GetBySessionId(stSession.m_dwSessionId);
                if (poPlayer)
                {
                    LOGERR("Player(%s) Uin(%lu) time is advanced than server, client_time(%lu), server_time(%lu)",
                        poPlayer->m_szName, poPlayer->m_ullUin, m_stCsRecvPkg.m_stHead.m_ullTimeStamp, ullCurTime);
                }

                this->SendToClient(&stSession, NULL, 0, PKGMETA::CONNSESSION_CMD_STOP);
            }
        }

		if (PKGMETA::CONNSESSION_CMD_START == stSession.m_chCmd)
        {
			poMsgHandler->HandleClientMsg(&stSession, m_stCsRecvPkg);
		}
        else if (PKGMETA::CONNSESSION_CMD_RECONN == stSession.m_chCmd)
        {
			poPlayer = PlayerMgr::Instance().GetByUin(m_stCsRecvPkg.m_stBody.m_stReconnectReq.m_ullUin);


            CpuSampleStats::Instance().BeginSample("CMsgLayerBase::_DealConnPkg::%hu", m_stSsRecvPkg.m_stHead.m_wMsgId);

            // poPlayer判空放到消息处理里面去
			poMsgHandler->HandleClientMsg(&stSession, m_stCsRecvPkg, poPlayer);

            CpuSampleStats::Instance().EndSample();
		}
        else if (PKGMETA::CONNSESSION_CMD_INPROC == stSession.m_chCmd)
        {
			// find player by session id, if not, notify ZoneConn, SESSION_STOP
			poPlayer = PlayerMgr::Instance().GetBySessionId(stSession.m_dwSessionId);
			if (!poPlayer)
            {
				//this->SendToClient(&stSession, NULL, PKGMETA::CONNSESSION_CMD_STOP);
			}
            else
			{

                CpuSampleStats::Instance().BeginSample("CMsgLayerBase::_DealConnPkg::%hu", m_stSsRecvPkg.m_stHead.m_wMsgId);

				poMsgHandler->HandleClientMsg(&stSession, m_stCsRecvPkg, poPlayer);

                CpuSampleStats::Instance().EndSample();

			}
		}
        else
        {
			assert(false);
		}
	}
}

int FightSvrMsgLayer::SendToClient(Player* poPlayer, PKGMETA::SCPKG* pstScPkg, char cSessCmd)
{
	if (!poPlayer)
    {
		assert(false);
		return -1;
	}

    pstScPkg->m_stHead.m_bProtoType = PKGMETA::PROTO_TYPE_UDP_R;
	return this->_SendToClient(m_iConnID, &poPlayer->m_stConnSession, pstScPkg, cSessCmd, poPlayer->GetProtocolVersion());
}

int FightSvrMsgLayer::SendToClient(const PKGMETA::CONNSESSION* pstConnSess, PKGMETA::SCPKG* pstScPkg, uint16_t wVersion, char cSessCmd)
{
    if (pstScPkg)
    {
        pstScPkg->m_stHead.m_bProtoType = PKGMETA::PROTO_TYPE_UDP_R;
    }

	return this->_SendToClient(m_iConnID, pstConnSess, pstScPkg, cSessCmd, wVersion);
}

int FightSvrMsgLayer::SendToClientWithoutAck(Player* poPlayer, PKGMETA::SCPKG* pstScPkg, char cSessCmd)
{
    if (!poPlayer)
    {
		assert(false);
		return -1;
	}

    pstScPkg->m_stHead.m_bProtoType = PKGMETA::PROTO_TYPE_UDP_N;
	return this->_SendToClient(m_iConnID, &poPlayer->m_stConnSession, pstScPkg, cSessCmd, poPlayer->GetProtocolVersion());
}

int FightSvrMsgLayer::SendToClientWithoutAck(const PKGMETA::CONNSESSION* pstConnSess, PKGMETA::SCPKG* pstScPkg, uint16_t wVersion, char cSessCmd)
{
    pstScPkg->m_stHead.m_bProtoType = PKGMETA::PROTO_TYPE_UDP_N;
    return this->_SendToClient(m_iConnID, pstConnSess, pstScPkg, cSessCmd, wVersion);
}

int FightSvrMsgLayer::SendToClusterGate(PKGMETA::SSPKG& rstSsPkg)
{
    return SendToServer(CFightSvr::Instance().GetConfig().m_iClusterGateID, rstSsPkg);
}

int FightSvrMsgLayer::SendToZoneSvr(PKGMETA::SSPKG& rstSsPkg)
{
	rstSsPkg.m_stHead.m_ullReservId |= (uint64_t)PROC_ZONE_SVR << 32;
	return SendToServer(CFightSvr::Instance().GetConfig().m_iClusterGateID, rstSsPkg);
}



