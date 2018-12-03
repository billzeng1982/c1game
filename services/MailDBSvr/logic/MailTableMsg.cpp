#include "MailTableMsg.h"
#include "MailPriTable.h"
#include "MailPubTable.h"
#include "LogMacros.h"
#include "strutil.h"
#include "../framework/MailDBSvrMsgLayer.h"
#include "../thread/DBWorkThread.h"

using namespace PKGMETA;

int CSsMailPriTableCreateReq::HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara )
{
    CDBWorkThread* poWorkThread = (CDBWorkThread*)pvPara;

    SS_PKG_MAIL_PRI_TABLE_CREATE_REQ& rstSsPkgReqBody = rstSsPkg.m_stBody.m_stMailPriTableCreateReq;
    if (0 == rstSsPkgReqBody.m_stData.m_stBaseInfo.m_ullUin)
    {
        LOGERR_r("param is err");
        return -1;
    }

    SSPKG stTempSspkg;

    stTempSspkg.m_stHead.m_wMsgId = SS_MSG_MAIL_PRI_TABLE_CREATE_RSP;
    stTempSspkg.m_stHead.m_iDstProcId = rstSsPkg.m_stHead.m_iSrcProcId;

    SS_PKG_MAIL_PRI_TABLE_CREATE_RSP& rstSsPkgRspBody = stTempSspkg.m_stBody.m_stMailPriTableCreateRsp;
    bzero(&rstSsPkgRspBody, sizeof(rstSsPkgRspBody));

    rstSsPkgRspBody.m_nErrNo = poWorkThread->GetMailPriTable().CreateData(rstSsPkgReqBody.m_stData);
    rstSsPkgRspBody.m_stData = rstSsPkgReqBody.m_stData;

    poWorkThread->SendPkg(stTempSspkg);

    return 0;
}


int CSsMailPriTableGetDataReq::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    CDBWorkThread* poWorkThread = (CDBWorkThread*)pvPara;

    SS_PKG_MAIL_PRI_TABLE_GET_DATA_REQ& rstSsPkgReqBody = rstSsPkg.m_stBody.m_stMailPriTableGetDataReq;
    if (0 == rstSsPkgReqBody.m_ullUin)
    {
        LOGERR_r("param is err");
        return -1;
    }

    SSPKG stTempSspkg = {0};

    stTempSspkg.m_stHead.m_wMsgId = SS_MSG_MAIL_PRI_TABLE_GET_DATA_RSP;
    stTempSspkg.m_stHead.m_iDstProcId = rstSsPkg.m_stHead.m_iSrcProcId;

    SS_PKG_MAIL_PRI_TABLE_GET_DATA_RSP& rstSsPkgRspBody = stTempSspkg.m_stBody.m_stMailPriTableGetDataRsp;
    rstSsPkgRspBody.m_ullTransTokenId = rstSsPkgReqBody.m_ullTransTokenId; // ActionTokenid
    rstSsPkgRspBody.m_stData.m_stBaseInfo.m_ullUin = rstSsPkgReqBody.m_ullUin;

    //如果不存在，则创建
    if (poWorkThread->GetMailPriTable().CheckExist(rstSsPkgReqBody.m_ullUin) == 0)
    {
        rstSsPkgRspBody.m_nErrNo = poWorkThread->GetMailPriTable().CreateData(rstSsPkgRspBody.m_stData);
    }
    //存在则从数据库中取
    else
    {
        rstSsPkgRspBody.m_nErrNo = poWorkThread->GetMailPriTable().GetData(rstSsPkgReqBody.m_ullUin, rstSsPkgRspBody.m_stData);
    }
    poWorkThread->SendPkg( stTempSspkg );
    return 0;
}


int CSsMailPriTableUptNtf::HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara )
{
    CDBWorkThread* poWorkThread = (CDBWorkThread*)pvPara;

    SS_PKG_MAIL_PRI_TABLE_UPDATE_NTF& rstSsPkgNtfBody = rstSsPkg.m_stBody.m_stMailPriTableUpdateNtf;
    if (0 == rstSsPkgNtfBody.m_stData.m_stBaseInfo.m_ullUin)
    {
        LOGERR_r("param is err");
        return -1;
    }

    poWorkThread->GetMailPriTable().SaveData(rstSsPkgNtfBody.m_stData);
    return 0;
}




int CSsMailPubTableGetDataReq::HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara )
{
    CDBWorkThread* poWorkThread = (CDBWorkThread*)pvPara;

    int iLimitStart = 0;
    int iLimitEnd = MAX_MAIL_BOX_PUBLIC_NUM -1;
    int iRet = 0;
    SSPKG stSsPkg = { 0 };
    stSsPkg.m_stHead.m_wMsgId = SS_MSG_MAIL_PUB_TABLE_GET_DATA_RSP;
    SS_PKG_MAIL_PUB_TABLE_GET_DATA_RSP& rstAddRsp = stSsPkg.m_stBody.m_stMailPubTableGetDataRsp;
    rstAddRsp.m_nErrNo = ERR_NONE;
    do
    {
        iRet = poWorkThread->GetMailPubTable().GetOnceData(stSsPkg.m_stBody.m_stMailPubTableGetDataRsp, iLimitStart, iLimitEnd);
        iLimitStart += MAX_MAIL_BOX_PUBLIC_NUM;
        iLimitEnd += MAX_MAIL_BOX_PUBLIC_NUM;
        if (iRet >= 0)
        {
            rstAddRsp.m_nErrNo = ERR_NONE;
            poWorkThread->SendPkg(stSsPkg);
        }
        else
        {
            rstAddRsp.m_nErrNo = iRet;
            poWorkThread->SendPkg(stSsPkg);
        }
        LOGRUN_r("Get PubMail Num<%d> ", iRet);
    } while (MAX_MAIL_BOX_PUBLIC_NUM == iRet);
    return 0;
}


int CSsMailPubTableDelNtf::HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara )
{
    CDBWorkThread* poWorkThread = (CDBWorkThread*)pvPara;

    SS_PKG_MAIL_PUB_TABLE_DEL_NTF& rstSsPkgNtfBody = rstSsPkg.m_stBody.m_stMailPubTableDelNtf;
    if (0 == rstSsPkgNtfBody.m_dwId)
    {
        LOGERR_r("param is err");
        return -1;
    }
    poWorkThread->GetMailPubTable().DelData(rstSsPkgNtfBody.m_dwId);
    return 0;
}



int CSsMailPubTableAddReq::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /*= NULL */)
{

    CDBWorkThread* poWorkThread = (CDBWorkThread*)pvPara;
    SS_PKG_MAIL_PUB_TABLE_ADD_REQ& rstSsPkgNtfBody = rstSsPkg.m_stBody.m_stMailPubTableAddReq;
    if (0 == rstSsPkgNtfBody.m_stPubMail.m_dwId)
    {
        LOGERR_r("param is err");
        return -1;
    }

    int iRet = poWorkThread->GetMailPubTable().AddData(rstSsPkgNtfBody.m_stPubMail);

    SSPKG stTempSspkg = {0};
    stTempSspkg.m_stHead.m_wMsgId = SS_MSG_MAIL_PUB_TABLE_GET_DATA_RSP;
    SS_PKG_MAIL_PUB_TABLE_GET_DATA_RSP& rstSsPkgBodyRsp = stTempSspkg.m_stBody.m_stMailPubTableGetDataRsp;
    rstSsPkgBodyRsp.m_nErrNo = iRet;
    rstSsPkgBodyRsp.m_wNum = 1;
    rstSsPkgBodyRsp.m_astPubMails[0] = rstSsPkgNtfBody.m_stPubMail;
    poWorkThread->SendPkg(stTempSspkg);
    return 0;
}
