#include "MsgLogicMasterSkill.h"
#include "LogMacros.h"
#include "common_proto.h"
#include "../gamedata/GameDataMgr.h"
#include "../framework/ZoneSvrMsgLayer.h"
#include "../module/player/Player.h"
#include "../module/player/PlayerMgr.h"
#include "../module/MasterSkill.h"
#include "../module/Task.h"

using namespace PKGMETA;

int MsgLogicMSUpgrade::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    CS_PKG_MS_UPGRADE_REQ& rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stMSUpgradeReq;

    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
	if (NULL == poPlayer)
	{
		LOGRUN("player not exist.");
		return -1;
	}

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_MS_UPGRADE_RSP;
    SC_PKG_MS_UPGRADE_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stMSUpgradeRsp;
	rstScPkgBodyRsp.m_bMSId = rstCsPkgBodyReq.m_bMSId;
	rstScPkgBodyRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;
	if (rstCsPkgBodyReq.m_bIsTotal == 1)
	{
		rstScPkgBodyRsp.m_nErrNo = MasterSkill::Instance().UpgradeMSTotal(&poPlayer->GetPlayerData(), rstScPkgBodyRsp);
	}
	else
	{
		rstScPkgBodyRsp.m_nErrNo = MasterSkill::Instance().UpgradeMS(&poPlayer->GetPlayerData(), rstScPkgBodyRsp);
	}
	
	if (rstScPkgBodyRsp.m_nErrNo == ERR_NONE)
	{
		//ÐÞ¸ÄÈÎÎñ
		Task::Instance().ModifyData(&poPlayer->GetPlayerData(), TASK_VALUE_TYPE_LVUP, 1, 5);
	}

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

int MsgLogicMSComposite::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    CS_PKG_MS_COMPOSITE_REQ& rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stMSCompositeReq;

    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (NULL == poPlayer)
    {
        LOGRUN("player not exist.");
        return -1;
    }

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_MS_COMPOSITE_RSP;
    SC_PKG_MS_COMPOSITE_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stMSCompositeRsp;
    rstScPkgBodyRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;
    rstScPkgBodyRsp.m_nErrNo = MasterSkill::Instance().Composite(&poPlayer->GetPlayerData(), rstCsPkgBodyReq.m_bMSId, rstScPkgBodyRsp);

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

