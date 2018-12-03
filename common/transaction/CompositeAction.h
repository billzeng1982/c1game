#ifndef _COMPOSITE_ACTION_H_
#define _COMPOSITE_ACTION_H_

#include "transaction.h"

#define MAX_COMPOSITE_ACTIONS 64

/*
	用于包装多个action并行执行, 不能继承使用
*/
class CompositeAction : public IAction
{
public:
	CompositeAction();
	virtual ~CompositeAction();

	virtual int ActionType() { return ACTION_TYPE_COMPOSITE; }

	//事务动作超时时间长度
    unsigned int GetTimeout() { return m_dwTimeout; }

	virtual int FiniFlag();

	/*执行事务动作
      返回值：
       0=所有执行已结束，无需等待执行结果
       1=同步执行结束，需等待异步结果返回
    */
    int Execute( Transaction* pObjTrans );

    virtual const char* GetActionName() { return "CompositeAction"; }

	//处理事务动作结果
    void OnAsyncRspMsg(TActionIndex dwActionIdx, const void* pResult, unsigned int dwResLen);

	//TActionIndex GetFirstIndex(){ return m_dwFirstIdx; }
    //TActionIndex GetLastIndex(){ return m_dwLastIdx; }

	int AddAction(IAction* pSubAction);
    IAction* GetAction(TActionIndex dwActionIdx);
	int GetActionCount() { return m_iActionCount;}

	virtual void Reset();

private:
	IAction* 	m_aActionArray[MAX_COMPOSITE_ACTIONS];  //子动作列表
	int 		m_iActionCount;

    	unsigned int  m_dwTimeout;
};

#endif

