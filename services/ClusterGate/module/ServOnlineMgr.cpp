#include "ServOnlineMgr.h"
#include "LogMacros.h"
#include "common_proto.h"
#include "ClusterGateMsgLayer.h"

using namespace PKGMETA;
using namespace std;

ServOnlineMgr::ServOnlineMgr()
{
    m_iCurFightSvrProcId = 0;
}

ServOnlineMgr::~ServOnlineMgr()
{

}

bool ServOnlineMgr::Init()
{
    m_stZoneSvrMap.clear();
    m_stFightSvrMap.clear();
    return true;
}

int ServOnlineMgr::HandleServStatNtf(DT_SERVER_INFO& rstServInfo)
{
    int iFuncId = GET_PROC_FUNC_BY_ID(rstServInfo.m_iProcId);
    //int iInstId = GET_PROC_INST_BY_ID(rstServInfo.m_iProcId);
    
    switch(iFuncId)
    {
        case PROC_ZONE_SVR:
        {
            _HandleZoneSvrStatNtf(rstServInfo);
            break;
        }
        case PROC_FIGHT_SVR:
        {
            _HandleFightSvrStatNtf(rstServInfo);
            break;
        }
        default:
        {
            LOGERR("function id is error.");
            return -1;
        }
    }
    
    return 0;
}

int ServOnlineMgr::_HandleZoneSvrStatNtf(DT_SERVER_INFO& rstServInfo)
{
    bool bOnline = (rstServInfo.m_bOnline == 1) ? true : false;
    if (bOnline)
    {
        // 在线通知
        map<int, DT_SERVER_INFO>::iterator iter = m_stZoneSvrMap.find(rstServInfo.m_iProcId);
        if (iter == m_stZoneSvrMap.end())
        {
            m_stZoneSvrMap.insert(pair<int, DT_SERVER_INFO>(rstServInfo.m_iProcId, rstServInfo));
        }
    }
    else
    {
        // 下线通知
        m_stZoneSvrMap.erase(rstServInfo.m_iProcId);
    }
    
    return 0;
}

int ServOnlineMgr::_HandleFightSvrStatNtf(DT_SERVER_INFO& rstServInfo)
{
    bool bOnline = (rstServInfo.m_bOnline == 1) ? true : false;
    if (bOnline)
    {
        // 在线通知
        map<int, DT_SERVER_INFO>::iterator iter = m_stFightSvrMap.find(rstServInfo.m_iProcId);
        if (iter == m_stFightSvrMap.end())
        {
            m_stFightSvrMap.insert(pair<int, DT_SERVER_INFO>(rstServInfo.m_iProcId, rstServInfo));
            LOGWARN("FightSvr Online: <%d> <%d> <%hu>,Total<%lu>", rstServInfo.m_iProcId, rstServInfo.m_iIp, rstServInfo.m_wPort, m_stFightSvrMap.size());
        }
        else
        {
            // 更新负载情况
            iter->second.m_dwLoad = rstServInfo.m_dwLoad;
        }
    }
    else
    {
        // 下线通知
        m_stFightSvrMap.erase(rstServInfo.m_iProcId);
        LOGWARN("FightSvr Offline: <%d> <%d> <%hu>,Total<%lu>", rstServInfo.m_iProcId, rstServInfo.m_iIp, rstServInfo.m_wPort, m_stFightSvrMap.size());
    }

    _NotifyFightSvrStatToMatchSvr();
    return 0;
}

void ServOnlineMgr::_NotifyFightSvrStatToMatchSvr()
{
    if (m_stFightSvrMap.empty())
    {
        return;
    }
    
    map<int, DT_SERVER_INFO>::iterator iter = m_stFightSvrMap.begin();
    map<int, DT_SERVER_INFO>::iterator iterMin = iter;
    uint32_t dwLoadMin = iter->second.m_dwLoad;
    iter++;
    
    for (; iter != m_stFightSvrMap.end(); iter++)
    {
        if (iter->second.m_dwLoad < dwLoadMin)
        {
            dwLoadMin = iter->second.m_dwLoad;
            iterMin = iter;
        }
    }

    if (m_iCurFightSvrProcId != iterMin->second.m_iProcId)
    {
        m_iCurFightSvrProcId = iterMin->second.m_iProcId;
        m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_MACTH_FIGHT_SERV_NTF;
        m_stSsPkg.m_stBody.m_stMatchFightServNtf.m_stServInfo = iterMin->second;
        ClusterGateMsgLayer::Instance().SendToMatchSvr(m_stSsPkg);
    }
    
    return;
}

//  测试用
int ServOnlineMgr::GetAnotherSvrProcId(int iProcId)
{
    map<int, DT_SERVER_INFO>::iterator iter = m_stFightSvrMap.begin();
    for (; iter != m_stFightSvrMap.end(); iter++)
    {
        if (iter->first != iProcId)
        {
            return iter->first;
        }
    }
    return m_iCurFightSvrProcId;
}

