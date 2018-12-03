#pragma once
#include "define.h"
#include "common_proto.h"
#include "comm/tlist.h"


class CloneBattleTeam
{
public:
    TLISTNODE           m_stDirtyListNode;          //脏数据链表头,用于回写数据库
    TLISTNODE           m_stTimeListNode;           //时间链表头,用于LRU算法
    
private:
    PKGMETA::DT_CLONE_BATTLE_TEAM_INFO m_stTeamInfo;
    uint32_t            m_dwAvrLi;                   //平均战力


public:
    CloneBattleTeam() {};
    ~CloneBattleTeam() {};
    uint64_t GetTeamId() { return m_stTeamInfo.m_ullId; }
    uint8_t GetBossType() { return m_stTeamInfo.m_bBossType; }
    uint64_t GetCreateTime() { return  m_stTeamInfo.m_ullCreateTime; }
    void Clear();

    //解包数据
    bool InitFromDB(IN PKGMETA::DT_CLONE_BATTLE_TEAM_DATA& rstData);

    //打包数据
    bool PackToData(OUT PKGMETA::DT_CLONE_BATTLE_TEAM_DATA& rstData);

    //获取数据
    void GetTeamInfo(OUT  PKGMETA::DT_CLONE_BATTLE_TEAM_INFO& rstTeamInfo);

    //创建
    bool InitTeamInfo(uint64_t ullTeamId, uint8_t bBossType, uint32_t dwBossId, PKGMETA::DT_CLONE_BATTLE_MATE_INFO& rstMateInfo);

    //更新成员信息
    void UptMateInfo(PKGMETA::DT_CLONE_BATTLE_MATE_INFO& rstMateInfo);

    //增加成员
    int AddMateInfo(PKGMETA::DT_CLONE_BATTLE_MATE_INFO& rstMateInfo);

    //退出队伍
    int QuitTeam(uint64_t ullUin);

    //能否解散
    bool CanDissolve();

    //胜利处理
    void HandleWin(uint64_t ullUin);

    //广播给队友通知
    void BroadcastMate(uint8_t bNtfType);

    //设置快速匹配标识
    void SetMatchType(uint8_t bMatchType);

    //获取快速匹配标识
    uint8_t GetMatchType();

    //队伍是否满员
    bool IsFull();
    
    //获取队伍平均战力
    uint32_t GetAvrLi() { return m_dwAvrLi; }

    //是否为队长
    bool IsCaptain(uint64_t ullUin) { return ullUin == m_stTeamInfo.m_ullCaptain; }
private:
    //修改平均战力 增加/删除/更新完调用
    //bType=1#增加|2#删除|3#更新
    void _ChangeAvrLi(uint32_t dwLi, uint8_t bType, uint32_t dwOldLi = 0);
private:
    PKGMETA::DT_CLONE_BATTLE_MATE_INFO* _FindMate(uint64_t ullUin);
};


