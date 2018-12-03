#include "oi_misc.h"
#include "LogMacros.h"
#include "DirHttpServer.h"
#include "ZoneSvrMgr.h"
#include "DirSvr.h"
#include "GameDataMgr.h"

DirHttpServer::DirHttpServer()
{

}

bool DirHttpServer::Init()
{
    DIRSVRCFG* pstConfig = &DirSvr::Instance().GetConfig();
    if (!m_oHttpServer.Init("0.0.0.0", (uint16_t)pstConfig->m_iHttpSvrPort))
    {
        LOGERR_r("Init DirHttpServer failed, port=(%d)", pstConfig->m_iHttpSvrPort);
        return false;
    }

    if (!m_oHttpServer.AddRequestCb(pstConfig->m_szGetSvrListUri, DirHttpServer::_HandleGetSvrList, this))
    {
        LOGERR_r("Add GetSvrList CB failed");
        return false;
    }

    if (!LoadData())
    {
        LOGERR_r("Load SvrList Data failed");
        return false;
    }

    return true;
}

bool DirHttpServer::Fini()
{
    return true;
}

void DirHttpServer::Update()
{
    m_oHttpServer.RecvAndHandle();
}


void DirHttpServer::_HandleGetSvrList(struct evhttp_request *req, void *arg)
{
    //只接受get请求
    if (evhttp_request_get_command(req) != EVHTTP_REQ_GET)
    {
		LOGERR_r("http type error");
		return;
	}

    DirHttpServer* poDirHttpSvr = (DirHttpServer*)arg;
    CHttpServer* poHttpServer = &(poDirHttpSvr->m_oHttpServer);

    DT_SERVER_LIST& rstSvrList = poDirHttpSvr->GetSvrList();

    size_t ullUsed;
	TdrError::ErrorType iRet = rstSvrList.pack(poDirHttpSvr->m_szBuffer, CHttpServer::HTTP_REQ_DATA_BUF_SIZE, &ullUsed);
	if( iRet != TdrError::TDR_NO_ERROR)
	{
		LOGERR_r("pack rstSvrList error! iRet=<%d> ", (int)iRet);
		return;
	}

	evbuffer_add(poHttpServer->m_pEvRspData, poDirHttpSvr->m_szBuffer, ullUsed);
	evhttp_send_reply(req, HTTP_OK, "OK", poHttpServer->m_pEvRspData);
    evbuffer_remove(poHttpServer->m_pEvRspData, poDirHttpSvr->m_szBuffer, CHttpServer::HTTP_REQ_DATA_BUF_SIZE);
}

bool DirHttpServer::LoadData()
{
    ResServerMgr_t& rstResServerMgr= CGameDataMgr::Instance().GetResServerMgr();
    int iSumCount = rstResServerMgr.GetResNum();

    m_stSvrList.m_bCount = 0;

    for (int i = 0; i < iSumCount; i++)
    {
        if (i >= MAX_NUM_ZONESVR)
        {
            LOGERR_r("the number of server in xlxs is larger than MAX_NUM_ZONESVR.");
            return false;
        }
        RESSERVER* pResServer = rstResServerMgr.GetResByPos(i);
        if (pResServer->m_bIsEnable == 0)
        {
            continue;
        }
        m_stSvrList.m_astSvrList[m_stSvrList.m_bCount].m_dwSvrId = pResServer->m_dwSvrId;
        m_stSvrList.m_astSvrList[m_stSvrList.m_bCount].m_wPort = pResServer->m_wPort;
        m_stSvrList.m_astSvrList[m_stSvrList.m_bCount].m_dwState = pResServer->m_bStatus;
        memcpy(m_stSvrList.m_astSvrList[m_stSvrList.m_bCount].m_szSvrName, pResServer->m_szSvrName, MAX_NAME_LENGTH);
        memcpy(m_stSvrList.m_astSvrList[m_stSvrList.m_bCount].m_szUrl, pResServer->m_szUrl, MAX_URL_NAME_LENGTH);
        inet_aton(pResServer->m_szIp, (struct in_addr*)&m_stSvrList.m_astSvrList[m_stSvrList.m_bCount].m_iIp);

        m_stSvrList.m_bCount++;
    }

    return true;
}

DT_SERVER_LIST& DirHttpServer::GetSvrList()
{
    return m_stSvrList;
}

