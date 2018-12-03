#pragma once
#include "define.h"
#include "singleton.h"
#include "common_proto.h"
#include "ss_proto.h"
#include <map>

/*
    CluserGate,MatchSvr,FightSvr,为全局服务器，都通过ClusterGate和ZoneSvr通信
*/

class ServOnlineMgr : public TSingleton<ServOnlineMgr>
{
public:
    ServOnlineMgr();
    virtual ~ServOnlineMgr();
    bool Init();

    int HandleServStatNtf(PKGMETA::DT_SERVER_INFO& rstServInfo);
    bool IsZoneSvrOnline(int iProcId);
    int GetAnotherSvrProcId(int iProcId);
private:
    int _HandleZoneSvrStatNtf(PKGMETA::DT_SERVER_INFO& rstServInfo);
    int _HandleFightSvrStatNtf(PKGMETA::DT_SERVER_INFO& rstServInfo);

    void _NotifyFightSvrStatToMatchSvr();
    
public:
    int m_iCurFightSvrProcId;	// 当前匹配使用的FightSvrId
    
public:
    std::map<int, PKGMETA::DT_SERVER_INFO> m_stZoneSvrMap;      //Map<SvrProId, SerInfo>
    std::map<int, PKGMETA::DT_SERVER_INFO> m_stFightSvrMap;     //Map<SvrProId, SerInfo>
    PKGMETA::SSPKG m_stSsPkg;
};

