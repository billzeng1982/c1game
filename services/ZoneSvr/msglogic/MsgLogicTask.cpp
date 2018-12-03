#include "MsgLogicTask.h"
#include "LogMacros.h"
#include "common_proto.h"
#include "../gamedata/GameDataMgr.h"
#include "../framework/ZoneSvrMsgLayer.h"
#include "../module/player/Player.h"
#include "../module/player/PlayerMgr.h"
#include "../module/Task.h"

using namespace PKGMETA;

int TaskDraw_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    CS_PKG_TASK_DRAW_REQ& rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stTaskDrawReq;
    
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
	if (!poPlayer)
	{
		LOGERR("player not exist.");
		return -1;
	}

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_TASK_DRAW_RSP;
    SC_PKG_TASK_DRAW_RSP& rstPkgBodyRsp = m_stScPkg.m_stBody.m_stTaskDrawRsp;
    rstPkgBodyRsp.m_nErrNo = Task::Instance().HandleDrawBonus(&poPlayer->GetPlayerData(), rstCsPkgBodyReq.m_dwTaskId, rstPkgBodyRsp);
    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    
    return 0;
}

int TaskFinalAward_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }
    CS_PKG_TASK_FINAL_AWARD_REQ& rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stTaskFinalAwardReq;
    SC_PKG_TASK_FINAL_AWARD_RSP& rstPkgBodyRsp = m_stScPkg.m_stBody.m_stTaskFinalAwardRsp;
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_TASK_FINAL_AWARD_RSP;
    rstPkgBodyRsp.m_nErrNo = Task::Instance().GetTaskFinalAward(&poPlayer->GetPlayerData(), rstCsPkgBodyReq.m_bType, rstPkgBodyRsp);
    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    return 0;
}


int TaskModify_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL*/)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }
    CS_PKG_TASK_MODIFY_REQ& rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stTaskModifyReq;
    SC_PKG_TASK_MODIFY_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stTaskModifyRsp;
    int iRet = ERR_NONE;
    //这里主要是  前台完成后台无法验证的任务
    //  目前是 SDK的一些分享任务等.
    if (rstCsPkgBodyReq.m_bValueType == TASK_VALUE_TYPE_CLIENT_MODIFY1)
    {
        iRet = Task::Instance().ModifyData(&poPlayer->GetPlayerData(), rstCsPkgBodyReq.m_bValueType, 1, rstCsPkgBodyReq.m_ValuePara[0],
            rstCsPkgBodyReq.m_ValuePara[1], rstCsPkgBodyReq.m_ValuePara[2]);
    }
    else
    {
        iRet = ERR_SYS;
        LOGERR("Uin<%lu> TaskValueType<%hhu> error.", poPlayer->GetUin(), rstCsPkgBodyReq.m_bValueType);
    }
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_TASK_MODIFY_RSP;
    rstScPkgBodyRsp.m_nErrNo = iRet;
    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    return 0;
}
