

#include "./MineDBMsg.h"
#include "../module/MineOreTable.h"
#include "../framework/MineDBSvrMsgLayer.h"
#include "../thread/DBWorkThread.h"


int MineGetOreDataReq_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /*= NULL*/)
{
    CDBWorkThread* poWorkThread = (CDBWorkThread*)pvPara;
    SS_PKG_MINE_GET_ORE_DATA_REQ& rstReq = rstSsPkg.m_stBody.m_stMineGetOreDataReq;
    SS_PKG_MINE_GET_ORE_DATA_RSP& rstRsp = m_stSsPkg.m_stBody.m_stMineGetOreDataRsp;
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_MINE_GET_ORE_DATA_RSP;
    rstRsp.m_ullTokenId = rstReq.m_ullTokenId;
    rstRsp.m_nErrNo = poWorkThread->GetMineOreTable().GetData(rstReq.m_OreUidList, rstReq.m_bOreCount, 
        rstRsp.m_astOreList, &rstRsp.m_bOreCount);
    poWorkThread->SendToMineSvr(m_stSsPkg);
    return 0;
}


int MineUptOreDataNtf_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /*= NULL*/)
{
    CDBWorkThread* poWorkThread = (CDBWorkThread*)pvPara;
    SS_PKG_MINE_UPT_ORE_DATA_NTF& rstNtf = rstSsPkg.m_stBody.m_stMineUptOreDataNtf;
    poWorkThread->GetMineOreTable().UptData(rstNtf.m_astOreList, rstNtf.m_bOreCount);
    return 0;
}

int MineDelOreDataNtf_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /*= NULL*/)
{
    CDBWorkThread* poWorkThread = (CDBWorkThread*)pvPara;
    SS_PKG_MINE_DEL_ORE_DATA_NTF& rstNtf = rstSsPkg.m_stBody.m_stMineDelOreDataNtf;
    poWorkThread->GetMineOreTable().DelData(rstNtf.m_OreUidList, rstNtf.m_bOreCount);
    return 0;
}


int MineGetPlayerDataReq_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /*= NULL*/)
{
    CDBWorkThread* poWorkThread = (CDBWorkThread*)pvPara;
    SS_PKG_MINE_GET_PLAYER_DATA_REQ& rstReq = rstSsPkg.m_stBody.m_stMineGetPlayerDataReq;
    SS_PKG_MINE_GET_PLAYER_DATA_RSP& rstRsp = m_stSsPkg.m_stBody.m_stMineGetPlayerDataRsp;
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_MINE_GET_PLAYER_DATA_RSP;
    rstRsp.m_ullTokenId = rstReq.m_ullTokenId;
    rstRsp.m_nErrNo = poWorkThread->GetMinePlayerTable().GetData(rstReq.m_ullPlayerUin, rstRsp.m_stPlayer);
    poWorkThread->SendToMineSvr(m_stSsPkg);
    return 0;
}

int MineUptPlayerDataNtf_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /*= NULL*/)
{
    CDBWorkThread* poWorkThread = (CDBWorkThread*)pvPara;
    SS_PKG_MINE_UPT_PLAYER_DATA_NTF& rstNtf = rstSsPkg.m_stBody.m_stMineUptPlayerDataNtf;
    poWorkThread->GetMinePlayerTable().UptData(rstNtf.m_astPlayerList[0]);
    return 0;
}
