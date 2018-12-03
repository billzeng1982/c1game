

#include "CloneBattleDBMsg.h"
#include "../module/CloneBattleTable.h"
#include "../framework/MiscSvrMsgLayer.h"
#include "../thread/DBWorkThread.h"



int CloneBattleDelDataNtf_SS::HandleServerMsg(PKGMETA::SSPKG & rstSsPkg, void * pvPara)
{
    CDBWorkThread* poWorkThread = (CDBWorkThread*)pvPara;
    SS_PKG_CLONE_BATTLE_DEL_DATA_NTF& rstNtf = rstSsPkg.m_stBody.m_stCloneBattleDelDataNtf; 
    if (rstNtf.m_dwCount == 0 || rstNtf.m_dwCount > sizeof(rstNtf.m_TeamIdList))
    {
        return 0;
    }
    poWorkThread->GetCloneBattleTable().DelData(rstNtf.m_TeamIdList, rstNtf.m_dwCount);
    return 0;
}

int CloneBattleUptDataNtf_SS::HandleServerMsg(PKGMETA::SSPKG & rstSsPkg, void * pvPara)
{
    CDBWorkThread* poWorkThread = (CDBWorkThread*)pvPara;
    SS_PKG_CLONE_BATTLE_UPT_DATA_NTF& rstNtf = rstSsPkg.m_stBody.m_stCloneBattleUptDataNtf;
    poWorkThread->GetCloneBattleTable().UptData(rstNtf.m_stCloneBattleData);
    return 0;
}

int CloneBattleGetDataReq_SS::HandleServerMsg(PKGMETA::SSPKG & rstSsPkg, void * pvPara)
{
    CDBWorkThread* poWorkThread = (CDBWorkThread*)pvPara;
    SS_PKG_CLONE_BATTLE_GET_DATA_REQ& rstReq = rstSsPkg.m_stBody.m_stCloneBattleGetDataReq;
    SS_PKG_CLONE_BATTLE_GET_DATA_RSP& rstRsp = m_stSsPkg.m_stBody.m_stCloneBattleGetDataRsp;
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_CLONE_BATTLE_GET_DATA_RSP;
    rstRsp.m_stCloneBattleData.m_stBaseInfo.m_ullId = rstReq.m_ullTeamId;
    rstRsp.m_ullTokenId = rstReq.m_ullTokenId;
    rstRsp.m_nError = poWorkThread->GetCloneBattleTable().GetData(rstRsp.m_stCloneBattleData);
    poWorkThread->SendToCloneBattleSvr(m_stSsPkg);
    return 0;
}


