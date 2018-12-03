#ifndef _TRANSACTION_H_
#define _TRANSACTION_H_

#include <time.h>
#include "define.h"
#include "DynArray.h"
#include "object.h"

/* 事务ID，由框架顺序分配，在一段时间内不会重复 */
typedef unsigned int TTransactionID;

/* 事务动作索引，用来标识事务对象中的动作对象，不可重复 */
typedef int TActionIndex;

/* 事务动作的token标识，由框架产生用来标识事务动作 */
typedef uint64_t TActionToken;

//===========================================================
// macro defs
#define DEFAULT_TRANC_TIME_OUT 5000
#define DEFAULT_ACTION_TIME_OUT 5000

#define ACTION_TYPE_SIMPLE 1
#define ACTION_TYPE_COMPOSITE 2

//===========================================================

class Transaction;


/*
	m_iFiniFlag - 1: 完成 0: 未完成,需要等待异步执行结果 <0: 出错
	出错则终止transaction执行
*/
class IAction : public IObject
{
public:
	IAction()
	{
		this->Reset();
	}

	virtual ~IAction() { m_iIndex = (TActionIndex)-1; }

	// CompositeAction override 此方法
	virtual int ActionType() { return ACTION_TYPE_SIMPLE; }

	void SetIndex(TActionIndex iIndex){ m_iIndex = iIndex; }
	TActionIndex GetIndex() const { return m_iIndex; }

	// 超时时间长度, 单位:毫秒
	uint32_t  GetTimeout() { return m_dwTimeout; }
	void SetTimeout( uint32_t dwTimeout ) { m_dwTimeout = dwTimeout; }

	/*
		0: 此动作还没执行或没执行结束（在等待异步执行结果）
		1: 此动作已经执行结束，且成功
		< 0: 此动作已经执行结束，且失败
	*/
	virtual int  FiniFlag() { return m_iFiniFlag; }
	void SetFiniFlag( int iVal ) { m_iFiniFlag = iVal; }

	// FiniFlag == 1时调用
	virtual void OnFinished( ) { }

    virtual const char* GetActionName() { return "action"; }

	/*
		执行动作，实现具体的动作操作和请求

    	return:  1 - 动作已经执行结束，没有异步结果需要等待
                     计算型动作或因为出错而不用等待异步执行结果的动作，在执行时应返回0
                     框架收到此返回值，会判断包含此动作的事务是否已经结束，如果没结束则继续执行后续动作
                 0 - 动作已经发出了异步请求，需要等待异步执行结果
                     框架收到此返回值，会把包含此动作的事务挂起，直到收到异步结果或超时
                 < 0 - error

	*/
	virtual int Execute( Transaction* pObjTrans ) { return 0; }

	/*
		处理异步返回结果数据包
		框架收到后台server返回的数据包时，用parser检查是否事务相关的数据包
		如果是，则交给事务内的动作进行处理

		para:
			iActionIdx 此动作的在事务中的索引(从0开始)
			pResult     后台server返回的异步结果数据包
			dwResLen    结果数据包长度
	*/
	virtual void OnAsyncRspMsg(TActionIndex iActionIdx, const void* pResult, unsigned int dwResLen) {}

	virtual void Reset()
	{
		m_iIndex = (TActionIndex)-1;
		m_iFiniFlag = 0;
		m_dwTimeout = 0;
		m_ullToken = 0;
	}

	void SetToken( TActionToken ullToken ) { m_ullToken = ullToken; }
	TActionToken GetToken() { return m_ullToken; }

protected:
	TActionIndex  m_iIndex;

	/*
		0: 此动作还没执行或没执行结束（在等待异步执行结果）
		1: 此动作已经执行结束，且成功
		< 0: 此动作已经执行结束，且失败
	*/
	int  m_iFiniFlag;

	uint32_t m_dwTimeout; // ms
	TActionToken m_ullToken;
};


/**  异步事务接口
 *   - 说明：
 *   -# 简称事务接口，此接口由应用实现，应用实现的事务类都必须从此接口派生
 *   -# 事务类是动作类的管理者和容器，框架使用 Transaction::GetAction 从事务类中取得具体动作并执行
 *   -# 框架使用 Transaction::GetAction 获取动作时，如果收到NULL指针，则认为没有更多动作要执行，将结束事务
 *   -# 事务类在执行过程中，可能由于内部动作需要等待异步结果而被挂起，直到等到异步结果或超时
 *   -# 事务在挂起过程中，框架会检测事务是否超时
 *   -# 事务在执行过程中，框架会调用 Transaction::IsFinished 判断事务是否执行结束
 *   -# 事务超时时，框架会调用 Transaction::OnFinished ( TRANC_ERR_TIMEOUT , ...)
 *   -# 事务成功结束时（事务内部错误导致事务结束，也认为是成功结束），框架会调用 Transaction::OnFinished ( TRANC_ERR_SUCCESS , ...)
 *   -# 成功加入框架的事务，框架会调用 IApp::OnReleaseTransaction 通知应用回收事务对象；加入失败时，需要应用自己释放事务对象
 *   -# Transaction::Init 和 Transaction::Fini 是为应用使用的接口，框架暂时不会使用
 *   - 要求：
 *   -# 事务成功加入框架后，不要在应用中继续操作此事务的指针，以免框架调用 IApp::OnReleaseTransaction 释放事务后引起奔溃
 *   - 技巧：
 *   -# 释放组合动作对象，可以在 TransactionFrame::OnReleaseTransaction 中进行
 */
class Transaction : public IObject
{
	const static int MAX_ACTION_NUM = 64;

public:
	Transaction();

	virtual ~Transaction(){ m_iActionCount = 0; }

	/**  \brief  获取事务名称，方便输出日志时使用
     *   \param  无
     *   \return 事务名称
     */
    virtual const char* GetTransactionName() { return "transaction"; }

	virtual void Reset();

	void SetTransactionID(TTransactionID dwTransactionID) { m_dwTransactionID = dwTransactionID; }
	TTransactionID GetTransactionID() { return m_dwTransactionID; }

	/**  \brief  添加动作至事务中
     *   \param  [in] pObjAction      动作对象
     *   \return TActionIndex         事务为动作分配的动作ID；返回-1时，此动作不能加入事务中
     */
    TActionIndex AddAction(IAction* poAction);

	/**  \brief  获取事务动作，通过索引号获取；索引超过最大个数时，返回NULL
     *   \param  [in] iIndex   动作索引
     *   \return IAction*      动作对象指针；返回NULL时，表示没有更多动作要执行了
     */
    IAction* GetAction(TActionIndex iIndex);

	// ms
	unsigned int GetTimeout() { return m_dwTimeout; }
	void SetTimeout( uint32_t dwTimeout ) { m_dwTimeout = dwTimeout; }

	bool IsFinished() { return m_bFinished; }

	/**  \brief  框架通知事务，某个动作结束了；方便事务做下一步初始化和流程安排
     *   \param  [in] iIndex       执行结束的动作索引
     *   \return 无
     */
    //virtual void OnActionFinish(TActionIndex iIndex){}

	/**   框架通知事务，整个事务已经结束了；方便事务做最终结果的处理
     */
    virtual void OnFinished( int iErrCode ){}

    virtual void OnActionFinished(TActionIndex iActionIdx) {}

	int GetActionCount() const
    {
        return m_iActionCount;
    }

	// 设置所有action的token
	void SetActionsToken();

    //设置某个Action的Token
    void SetActionToken(TActionIndex iIndex);

	int CurStep() { return m_iCurStep; }
	void NextStep();

protected:
	int m_iActionCount;

	TTransactionID	m_dwTransactionID;
	bool m_bFinished;
	uint32_t m_dwTimeout; // ms

	int m_iCurStep;

	IAction* m_aActionArray[MAX_ACTION_NUM];
};

#endif

