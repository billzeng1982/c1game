#pragma once
#ifndef HTTP_CONTROLER_H_
#define HTTP_CONTROLER_H_

#include "http/HttpServer.h"
#include "cfg/ZoneHttpConnCfgDesc.h"
#include "protocol/PKGMETA/ss_proto.h"
#include "utils/MyTdrBuf.h"
#include "module/HttpReqTimer.h"
#include "GameTimerMgr_PQ.h"
#include <map>

class HttpControler
{
    typedef std::map<uint32_t, struct evhttp_request*> HttpReqMap_t;
    typedef std::map<uint32_t, uint32_t> HttpReqTimerIdMap_t;
    static const int DEFAULT_HTTP_REQ_TIMER_NUM;
    static const int HTTP_REQ_TIMER_TIMEOUT_TIME;
public:
    HttpControler();
    ~HttpControler() {}

    bool Init(ZONEHTTPCONNCFG& rstConfig);
    void Update();

    CHttpServer* GetHttpServer() {return &m_oHttpServer;}

    //收到获取玩家网页任务状态的请求时的回调函数
    void OnHttpGetRoleWebTaskInfo(struct evhttp_request *req, char* szReqJson);
    //收到领取网页任务奖励的post请求时的回调函数
    void OnHttpGetWebTaskRwd(struct evhttp_request *req, char* szReqJson);
    //收到领取网页任务奖励的get请求时的回调函数
    void OnHttpGetWebTaskRwd(struct evhttp_request *req, PKGMETA::SS_PKG_WEB_TASK_GET_RWD_REQ& rstSsReq);

    //释放定时器
    static void ReleaseTimer(GameTimer* pTimer);

    //发送回包给http服务器
    void SendHttpRsp(PKGMETA::SSPKG& rstSsPkg);
    //发送超时包给http服务器
    void SendTimeoutRsp(PKGMETA::SSPKG& rstSsPkg);

    //删除对应的请求
    bool DeleteReq(uint32_t dwReqId);
    //删除定时器
    bool DeleteTimer(uint32_t dwReqId);

    //发送玩家网页任务状态给http服务器
    void SendGetRoleWebTaskInfoHttpRsp(uint32_t dwReqId, PKGMETA::SS_PKG_ROLE_WEB_TASK_INFO_RSP& rstRoleWebTaskInfoRsp);

    //发送玩家领取网页任务奖励的结果给http服务器
    void SendGetRoleWebTaskRwdHttpRsp(uint32_t dwReqId, PKGMETA::SS_PKG_WEB_TASK_GET_RWD_RSP& rstWebTaskGetRwdRsp);

private:
    uint32_t _GetNextReqId() { return ++m_dwReqId == 0 ? ++m_dwReqId : m_dwReqId; }
    struct evhttp_request* _FindReq(uint32_t dwReqId);
    //将请求保存到map中
    void _AddToHttpReqMap(uint32_t dwReqId, struct evhttp_request *req);
    //将请求对应的定时器保存到map中
    void _AddToHttpTimerMap(uint32_t dwReqId, uint32_t dwTimerId);

    //发送玩家网页任务状态超时回复给http服务器
    void _SendGetRoleWebTaskInfoHttpTimeoutRsp(uint32_t dwReqId, PKGMETA::SS_PKG_ROLE_WEB_TASK_INFO_RSP& rstRoleWebTaskInfoRsp);
    //发送玩家领取网页任务奖励超时的结果给http服务器
    void _SendGetRoleWebTaskRwdHttpTimeoutRsp(uint32_t dwReqId, PKGMETA::SS_PKG_WEB_TASK_GET_RWD_RSP& rstWebTaskGetRwdRsp);

    void _SendErrRsp(struct evhttp_request *req, int iErrNo);
    void _SendHttpRsp(struct evhttp_request *req, char* szRspStr);

private:
    CHttpServer m_oHttpServer;
    ZONEHTTPCONNCFG m_stConfig;
    LPTDRMETALIB m_pstMetaLib;
    PKGMETA::SSPKG m_stSsPkg;
    uint32_t m_dwReqId;
    HttpReqMap_t m_oHttpReqMap;
    //ReqId -> HttpTimerId
    HttpReqTimerIdMap_t m_oHttpTimerMap;
    GameTimerMgr_PQ m_oTimerMgr;
    MyTdrBuf m_stTdrBuf;
    char m_szJsonRsp[4096];
};

#endif
