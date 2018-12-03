#include "LogMacros.h"
#include "ObjectPool.h"
#include "ClusterGateMsgLayer.h"
#include "../ClusterGate.h"
#include "../module/ServOnlineMgr.h"
#include "../msglogic/MsgLogicClusterGate.h"
#include "define.h"
using namespace PKGMETA;

ClusterGateMsgLayer::ClusterGateMsgLayer() : CMsgLayerBase(sizeof(PKGMETA::SCPKG) + sizeof(PKGMETA::ConnSession) + 1,
                                                                sizeof(PKGMETA::SSPKG) + sizeof(PKGMETA::ConnSession) + 1)
{

}

void ClusterGateMsgLayer::_RegistServerMsg()
{
    REGISTER_MSG_HANDLER(SS_MSG_CLUSTER_SERV_STAT_NTF,       ClusterServStatNtf_SS, m_oSsMsgHandlerMap);
}

bool ClusterGateMsgLayer::Init()
{
    CLUSTERGATECFG& rConfig = ClusterGate::Instance().GetConfig();
    if (!this->_Init(rConfig.m_iBusGCIMKey, rConfig.m_iProcID))
    {
        return false;
    }

    return true;
}

int ClusterGateMsgLayer::DealPkg()
{
    int iRecvBytes = 0;
    int i = 0;

    for (; i < DEAL_PKG_PER_LOOP; i++)
    {
        iRecvBytes = m_oCommBusLayer.Recv();
        if (iRecvBytes < 0)
        {
            LOGERR("bus recv error: %s", m_oCommBusLayer.GetErrMsg());
            return -1;
        }

        if (0 == iRecvBytes)
        {
            break;
        }

        this->_DealSvrPkg();
    }

    return i;
}

bool ClusterGateMsgLayer::_ForwardPkg(MyTdrBuf* pstTdrBuf)
{
    if (!pstTdrBuf)
    {
        return false;
    }

    int iMsgId = SSPKG_MSGID(pstTdrBuf->m_szTdrBuf);

    if (iMsgId == SS_MSG_MATCH_START_REQ)
    {
        LOGRUN("msg id -%d", iMsgId);
    }

    CLUSTERGATECFG& rstConfig = ClusterGate::Instance().GetConfig();

	uint64_t ullReservId = SSPKG_RESERVID(pstTdrBuf->m_szTdrBuf);
	//低32位表示 功能服务器通信地址
	//高32位表示 功能服务器区分ID 通信地址的左起第3个字节
	int iServProcId =	0xFFFFFFFF & ullReservId;
	int iFuncId =		ullReservId >> 32;
    
	//由 iFuncId 决定转发给谁
	switch (iFuncId)
	{
	case PROC_MINE_SVR:
		SendToMineSvr(pstTdrBuf);
		return true;
	case PROC_ZONE_SVR:
	case PROC_PASS_THROUGH:
		SendToServer(iServProcId, pstTdrBuf);
		return true;
	case PROC_SERIAL_NUM_SVR:
		SendToSerialNumSvr(pstTdrBuf);
		return true;
	case PROC_MATCH_SVR:
		SendToMatchSvr(pstTdrBuf);
		return true;
	case PROC_GUILD_EXPEDITION_SVR:
		SendToGuildExpeditionSvr(pstTdrBuf);
        return true;
	case PROC_FIGHT_SVR:
	{
		// 由Cluster来做负载均衡，分配负载最轻的FightSvr
		int iFightProcId = ServOnlineMgr::Instance().m_iCurFightSvrProcId;

		/*/ 测试多FightSvr进程 ......
		static int iTestCurProcId = ServOnlineMgr::Instance().m_iCurFightSvrProcId;
		iServProcId = ServOnlineMgr::Instance().GetAnotherSvrProcId(iTestCurProcId);
		LOGWARN("#### The selected Proc is <%d> ",iServProcId);
		iTestCurProcId = iServProcId;
		//*/

		SendToServer(iFightProcId, pstTdrBuf);
		return true;
	}
    case PROC_CLUSTER_ACCOUNT_SVR:
        SendToServer(rstConfig.m_iClusterAccountSvrID, pstTdrBuf);
        return true;
	default:
		break;
	}

    return false;
}

void ClusterGateMsgLayer::_DealSvrPkg()
{
    MyTdrBuf* pstTdrBuf = m_oCommBusLayer.GetRecvTdrBuf();

	// 对特定包执行转发, 注意转发包不要去解包
	if (_ForwardPkg(pstTdrBuf))
	{
		return;
	}

    TdrError::ErrorType iRet = m_stSsRecvPkg.unpack(pstTdrBuf->m_szTdrBuf, pstTdrBuf->m_uPackLen);
    if( iRet != TdrError::TDR_NO_ERROR)
    {
        return;
    }
    
    IMsgBase* poMsgHandler = this->GetServerMsgHandler( m_stSsRecvPkg.m_stHead.m_wMsgId );
    if( poMsgHandler )
    {
        poMsgHandler->HandleServerMsg( m_stSsRecvPkg );
        return;
    }
	//未绑定消息Id 回传?
	LOGERR("SrcSvrId<%d>  MsgId<%d>  not find!", (int)m_stSsRecvPkg.m_stHead.m_iSrcProcId, (int)m_stSsRecvPkg.m_stHead.m_wMsgId);
    this->SendToServer(m_stSsRecvPkg.m_stHead.m_iDstProcId, m_stSsRecvPkg);
}

int ClusterGateMsgLayer::SendToMatchSvr(PKGMETA::SSPKG& rstSsPkg)
{
    return SendToServer(ClusterGate::Instance().GetConfig().m_iMatchSvrID, rstSsPkg);
}

int ClusterGateMsgLayer::SendToMatchSvr(MyTdrBuf* pstTdrBuf)
{
    return SendToServer(ClusterGate::Instance().GetConfig().m_iMatchSvrID, pstTdrBuf);
}

int ClusterGateMsgLayer::SendToSerialNumSvr(MyTdrBuf* pstTdrBuf)
{
    return SendToServer(ClusterGate::Instance().GetConfig().m_iSerialNumSvrID, pstTdrBuf);
}

int ClusterGateMsgLayer::SendToMineSvr(MyTdrBuf* pstTdrBuf)
{
	return SendToServer(ClusterGate::Instance().GetConfig().m_iMineSvrID, pstTdrBuf);
}

int ClusterGateMsgLayer::SendToGuildExpeditionSvr(MyTdrBuf* pstTdrBuf)
{
	return SendToServer(ClusterGate::Instance().GetConfig().m_iGuildExpeditionSvrID, pstTdrBuf);
}
