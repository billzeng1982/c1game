#pragma once
#include <map>
#include <vector>

#include "define.h"
#include "common_proto.h"
#include "cs_proto.h"
#include "singleton.h"
#include "PriorityQueue.h"
#include "player/PlayerData.h"
#include "../gamedata/GameDataMgr.h"

class TaskAct : public TSingleton<TaskAct>
{

public:
    const static uint32_t ACT_OPEN_TYPE_PRIVATE = 1;  //私有开启类型
public:
    TaskAct() {};
    ~TaskAct() {};
    bool Init();
    void Update();
    void UpdateActState();
    void UpdatePlayer(PlayerData* pstPData);

    //  检查公共开启活动
    void UpdatePublicAct(PlayerData* pstPData);  

    //  检查私有开启活动
    void UpdatePrivateAct(PlayerData* pstPData);    

    //判断是否有某种活动类型的活动开启
    bool IsActTypeOpen(PlayerData* pstPData, uint32_t dwType);  
    bool IsActTypeOpen(PlayerData* pstPData, uint32_t dwType, OUT uint64_t& rStartTime, OUT uint64_t& rEndTime);
    //**  4Player的接口主要是为了避免 极端情况下
    //活动开启后,玩家的相关数据还没重置,就开始进行活动 



    DT_ACT_INFO* AddAct(PlayerData* pstPData, uint32_t dwId);
    DT_ACT_INFO* FindAct(PlayerData* pstPData, uint32_t dwActId);

    //  获取活动表中的参数
    uint32_t GetActPara(uint32_t dwId);

    //  获取活动状态 返回是否活动是否开启的状态,如果活动未开启, rStartTime 和 rEndTime 是无效值
    bool GetActState(PlayerData* pstPData, uint32_t dwActId, OUT uint64_t& rStartTime, OUT uint64_t& rEndTime);

    //活动开启重置
    void ActOpenDo(PlayerData* pstPData, DT_ACT_INFO* pstActInfo, DT_ACT_TIME* pstActTime);
private:
    bool CheckActOpen(RESPAYACT* poResPayAct, uint64_t ullPara, OUT uint64_t& rStartTime, OUT uint64_t& rEndTime);
private:
    map<uint32_t, DT_ACT_TIME>                  m_ActId2ActTimeMap;         //<活动Id, 活动时间>
    map<uint32_t, DT_ACT_TIME>::iterator    m_ActIdActTimeMapIter;
    map<uint32_t, vector<uint32_t> >        m_ActType2ActIdsMap;        //<活动类型, 活动Id数组>
    vector<uint32_t>                        m_OpenedActIdVector;        //开放过的活动Id
    uint64_t                                m_ullLastCheckTime;
    set<uint32_t>                           m_CurOpenActIdSet;             //当前开放的活动Id
    uint64_t                                m_ullActLastUpdateTime;     //上次更新时间 活动有开放或关闭就会更新
    set<uint32_t>                           m_PravieActSet;             //私有活动

};




