#pragma once
#include "singleton.h"
#include "mempool.h"
#include "DynMempool.h"
#include "../cfg/CloneBattleSvrCfgDesc.h"
#include "comm/tlist.h"
#include "CoDataFrame.h"
#include "CloneBattleTeam.h"
#include "ss_proto.h"
#include "common_proto.h"
#include "../framework/CloneBattleSvrMsgLayer.h"
#include "CloneBattleMatch.h"

#define  FILE_NAME "CloneBattleFile"

class AsyncGetDataAction : public CoGetDataAction
{
public:
    AsyncGetDataAction() { m_ullTeamId = 0; }
    virtual ~AsyncGetDataAction() {}

    virtual bool Execute()
    {
        PKGMETA::SSPKG& rstSsPkg = CloneBattleSvrMsgLayer::Instance().GetSsPkgData();
        rstSsPkg.m_stHead.m_wMsgId = PKGMETA::SS_MSG_CLONE_BATTLE_GET_DATA_REQ;

        PKGMETA::SS_PKG_CLONE_BATTLE_GET_DATA_REQ& rstReq = rstSsPkg.m_stBody.m_stCloneBattleGetDataReq;
        rstReq.m_ullTeamId = m_ullTeamId;
        rstReq.m_ullTokenId = this->GetToken();
        CloneBattleSvrMsgLayer::Instance().SendToMiscSvr(rstSsPkg);
        return true;
    }
    void SetTeamId(uint64_t ullTeamId) { m_ullTeamId = ullTeamId; }

private:
    uint64_t m_ullTeamId;
};

struct FileData 
{
    uint32_t                                    m_dwActBossId;              //特殊BossId;
    uint64_t                                    m_ullActBeginTime;          //特殊boss活动开启时间,
    uint64_t                                    m_ullActEndTime;            //特殊boss活动结束时间,
    PKGMETA::DT_CLONE_BATTLE_BOSS_INFO          m_oBossInfo;                //随机出来的Boss信息
};

class CloneBattleMgr : public TSingleton<CloneBattleMgr>, public CoDataFrame
{
public:
    static const int CONST_HIDE_POOL_ID = 9999;
    static const int CONST_MATCH_TYPE_QUICK_ON = 1;         //可以快速匹配
    static const int CONST_MATCH_TYPE_QUICK_OFF = 0;        //禁止快速匹配
    
    typedef std::map<uint64_t, CloneBattleTeam*> Id2Team_t;

public:
    CloneBattleMgr() {};
    ~CloneBattleMgr() {};

    bool Init(CLONEBATTLESVRCFG* pstConfig);
    void Update(bool bIdle);
    void Fini();

    CloneBattleTeam* GetTeamInfo(uint64_t ullTeamId);

    //(继承自CoDataFrame,由继承的类具体实现) 判断数据是否在内存中
    virtual bool IsInMem(void* pResult);

    //(继承自CoDataFrame,由继承的类具体实现) 将结果保存在内存中
    virtual bool SaveInMem(void* pResult);

    //增加到Time表
    void AddToTimeList(CloneBattleTeam* pstTeamInfo);

    //增加到Dirty表
    void AddToDirtyList(CloneBattleTeam* pstTeamInfo);

    //移动到TimeList的首个
    void MoveToTimeListFirst(CloneBattleTeam* pstTeamInfo);

    //从Time表删除
    void DelFromTimeList(CloneBattleTeam* pstTeamInfo);

    //从Dirty表删除
    void DelFromDirtyList(CloneBattleTeam* pstTeamInfo);
protected:
    void _WriteDirtyToDB();

    //保存到数据库
    void _SaveTeamInfo(CloneBattleTeam* pstTeamInfo);

    //从数据库中删除
    void _DelTeamInfo(CloneBattleTeam* pstTeamInfo);


    //CloneBattleTeam* _NewTeamInfo(PKGMETA::DT_CLONE_BATTLE_TEAM_DATA& rstData);
    CloneBattleTeam* _GetNewTeamInfo();                             //获取一个新节点
    void _AddTeamInfoToMap(CloneBattleTeam* pstTeamInfo);
    void _DelTeamInfoFromMap(CloneBattleTeam* pstTeamInfo);

    //(继承自CoDataFrame,由继承的类具体实现) 通过key在内存中查找数据
	virtual void* _GetDataInMem(void* key);

    //(继承自CoDataFrame,由继承的类具体实现) 创建获取数据action
	virtual CoGetDataAction* _CreateGetDataAction(void* key);

    //(继承自CoDataFrame,由继承的类具体实现) 释放获取数据的action
	virtual void _ReleaseGetDataAction(CoGetDataAction* poAction);

 public:
    //获取Boss信息
    void GetBossInfo(PKGMETA::DT_CLONE_BATTLE_BOSS_INFO& rstBossInfo);

    //随机Boss 
    void RandBoss();

    //创建队伍
    CloneBattleTeam* CreateTeam(PKGMETA::SS_PKG_CLONE_BATTLE_JOIN_TEAM_REQ& rstReq);

    //匹配寻找队伍
    CloneBattleTeam* MatchTeam(uint64_t ullUin, PKGMETA::SS_PKG_CLONE_BATTLE_JOIN_TEAM_REQ& rstReq, int& rRet);

    //解散队伍
    int DissolveTeam(CloneBattleTeam* pstTeamInfo, bool bBroadcast = false, bool bSendDelTag = true);

    //到刷新时间点解散上个周期的队伍
    void UpdateToDissolve(bool bIdle);


    //创建一个队伍Id
    uint64_t CreateTeamId();

    //发送克隆战系统信息到ZoneSvr
    void SendSysInfo();

    void AddToMatchList(CloneBattleTeam* pstTeam);
    void DelFromMatchList(CloneBattleTeam* pstTeam);


    uint32_t GetMateFinishNum() { return m_dwMateFinishNum; }
    uint32_t GetTeamFinishNum() { return m_dwTeamFinishNum; }
private:
    PKGMETA::DT_CLONE_BATTLE_BOSS_INFO          m_oBossInfo;                //随机出来的Boss信息
    int                                         m_iUptHour;
    uint64_t                                    m_ullLastUptTimeMs;           //上次更新时间
    CloneBattleMatch                            m_arrMatch[MAX_NUM_CLONE_BATTLE_BOSS];    //每个BossType的匹配列表
    uint64_t                                    m_ullLastCreateUinTime;
    uint64_t                                    m_ullActBeginTime;          //特殊boss活动开启时间,
    uint64_t                                    m_ullActEndTime;            //特殊boss活动结束时间,
    uint32_t                                    m_dwSeq;
    uint32_t                                    m_dwMateFinishNum;          //单个玩家最大完成的胜利次数,
    uint32_t                                    m_dwTeamFinishNum;          //团队奖励需要完成的最低成员人数          
    FILE*                                       m_fp;                       //主要存Boss信息
private:
    CLONEBATTLESVRCFG*                          m_pstConfig;

    TLISTNODE                                   m_stDirtyListHead;          //脏数据链表头,用于回写数据库
    TLISTNODE                                   m_stTimeListHead;           //时间链表头,用于LRU算法
    uint32_t                                    m_dwDirtyListSize;          //脏数据链表长度
    uint32_t                                    m_dwTimeListSize;           //时间链表长度长度
    uint32_t                                    m_dwWriteTimeVal;           //回写的时间间隔
    uint64_t                                    m_ullLastWriteTimestamp;    //上次回写脏数据的时间

    uint32_t                                    m_dwCurCacheSize;
    uint32_t                                    m_dwMaxCacheSize;

    Id2Team_t                                   m_oTeamMap;
    Id2Team_t::iterator                         m_oTeamMapIter;

    DynMempool<AsyncGetDataAction>              m_oAsyncActionPool;
    CMemPool<CloneBattleTeam>                   m_oTeamPool;
    CMemPool<CloneBattleTeam>::UsedIterator     m_oTeamPoolIter;
};


