#include "GameEvent.h"

GameEventDispatcher::GameEventDispatcher()
{
    m_oEventIDCallBackMap.clear();
    m_oPool.Init(POOL_INIT_NUM, POOL_DELTA_NUM, POOL_MAX_NUM);
}

GameEventDispatcher::~GameEventDispatcher()
{
    this->UnRegisterAll();
}

void GameEventDispatcher::Register(int iEventID, int iPriority, CallBackFun& f)
{
    CallBackFunQueue* poQueue = NULL;

    map<int, CallBackFunQueue*>::iterator Iter;
    Iter = m_oEventIDCallBackMap.find(iEventID);
    if (Iter != m_oEventIDCallBackMap.end())
    {
        poQueue = Iter->second;
    }
    else
    {
        poQueue = new CallBackFunQueue();
        m_oEventIDCallBackMap.insert(map<int, CallBackFunQueue*>::value_type(iEventID, poQueue));
    }

    CallBackFunNode* stNode = m_oPool.Get();
    stNode->Set(iPriority, f);
    poQueue->Push(stNode);
}

void GameEventDispatcher::Dispatch(GameEvent* poEvent)
{
    if (!poEvent)
    {
        return;
    }

    map<int, CallBackFunQueue*>::iterator Iter;
    Iter = m_oEventIDCallBackMap.find(poEvent->m_iEventID);
    if (Iter != m_oEventIDCallBackMap.end())
    {
        CallBackFunQueue* poQueue = Iter->second;
        while (!poQueue->Empty())
        {
            CallBackFunNode* stNode = poQueue->Top();
            CallBackFun& f = stNode->f_;
            f(poEvent);
            poQueue->Pop();
            m_oPool.Release(stNode);
        }
    }
}

void GameEventDispatcher::UnRegisterAll()
{
    map<int, CallBackFunQueue*>::iterator Iter;
    for (Iter=m_oEventIDCallBackMap.begin(); Iter!=m_oEventIDCallBackMap.end(); Iter++)
    {
        CallBackFunQueue* poQueue = Iter->second;
        while (!poQueue->Empty())
        {
            m_oPool.Release(poQueue->Top());
            poQueue->Pop();
        }
    }
}


