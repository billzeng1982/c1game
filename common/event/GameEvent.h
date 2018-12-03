#pragma once
#include "object.h"
#include "singleton.h"
#include "functional.h"
#include "PriorityQueue.h"
#include <map>
#include <stdio.h>
#include "DynMempool.h"

using namespace std;
using namespace RayE;

enum GAME_EVENT_ID
{
    GAME_EVENT_TEST = 1,
};

class GameEvent
{
public:
    GameEvent(int iEventID) : m_iEventID(iEventID)
    {
    }
    virtual ~GameEvent() {}

public:
    int m_iEventID;
};

class GameEventDispatcher : public TSingleton<GameEventDispatcher>
{
private:
    static const int POOL_INIT_NUM = 100;
    static const int POOL_DELTA_NUM = 20;
    static const int POOL_MAX_NUM = 1000;

private:
    typedef Function1<void, GameEvent*> CallBackFun;

    class CallBackFunNode
    {
    public:

        void Set(int iPriority, CallBackFun& f)
        {
            iPriority_ = iPriority;
            f_ = f;
        }

        int iPriority_;
        CallBackFun f_;
    };

    struct Compare
    {
        bool operator() (CallBackFunNode*& lNode, CallBackFunNode*& rNode)
        {
            return lNode->iPriority_ < rNode->iPriority_;
        }
    };

    typedef PriorityQueue<CallBackFunNode*, Compare> CallBackFunQueue;

public:
    GameEventDispatcher();

    ~GameEventDispatcher();

    void Register(int iEventID, int iPriority, CallBackFun& f);

    void UnRegisterAll();

    void Dispatch(GameEvent* poEvent);

private:
    map<int, CallBackFunQueue*> m_oEventIDCallBackMap;
    DynMempool<CallBackFunNode> m_oPool;
};
