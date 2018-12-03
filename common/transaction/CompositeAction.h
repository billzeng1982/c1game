#ifndef _COMPOSITE_ACTION_H_
#define _COMPOSITE_ACTION_H_

#include "transaction.h"

#define MAX_COMPOSITE_ACTIONS 64

/*
	���ڰ�װ���action����ִ��, ���ܼ̳�ʹ��
*/
class CompositeAction : public IAction
{
public:
	CompositeAction();
	virtual ~CompositeAction();

	virtual int ActionType() { return ACTION_TYPE_COMPOSITE; }

	//��������ʱʱ�䳤��
    unsigned int GetTimeout() { return m_dwTimeout; }

	virtual int FiniFlag();

	/*ִ��������
      ����ֵ��
       0=����ִ���ѽ���������ȴ�ִ�н��
       1=ͬ��ִ�н�������ȴ��첽�������
    */
    int Execute( Transaction* pObjTrans );

    virtual const char* GetActionName() { return "CompositeAction"; }

	//�������������
    void OnAsyncRspMsg(TActionIndex dwActionIdx, const void* pResult, unsigned int dwResLen);

	//TActionIndex GetFirstIndex(){ return m_dwFirstIdx; }
    //TActionIndex GetLastIndex(){ return m_dwLastIdx; }

	int AddAction(IAction* pSubAction);
    IAction* GetAction(TActionIndex dwActionIdx);
	int GetActionCount() { return m_iActionCount;}

	virtual void Reset();

private:
	IAction* 	m_aActionArray[MAX_COMPOSITE_ACTIONS];  //�Ӷ����б�
	int 		m_iActionCount;

    	unsigned int  m_dwTimeout;
};

#endif

