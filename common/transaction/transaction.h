#ifndef _TRANSACTION_H_
#define _TRANSACTION_H_

#include <time.h>
#include "define.h"
#include "DynArray.h"
#include "object.h"

/* ����ID���ɿ��˳����䣬��һ��ʱ���ڲ����ظ� */
typedef unsigned int TTransactionID;

/* ������������������ʶ��������еĶ������󣬲����ظ� */
typedef int TActionIndex;

/* ��������token��ʶ���ɿ�ܲ���������ʶ������ */
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
	m_iFiniFlag - 1: ��� 0: δ���,��Ҫ�ȴ��첽ִ�н�� <0: ����
	��������ֹtransactionִ��
*/
class IAction : public IObject
{
public:
	IAction()
	{
		this->Reset();
	}

	virtual ~IAction() { m_iIndex = (TActionIndex)-1; }

	// CompositeAction override �˷���
	virtual int ActionType() { return ACTION_TYPE_SIMPLE; }

	void SetIndex(TActionIndex iIndex){ m_iIndex = iIndex; }
	TActionIndex GetIndex() const { return m_iIndex; }

	// ��ʱʱ�䳤��, ��λ:����
	uint32_t  GetTimeout() { return m_dwTimeout; }
	void SetTimeout( uint32_t dwTimeout ) { m_dwTimeout = dwTimeout; }

	/*
		0: �˶�����ûִ�л�ûִ�н������ڵȴ��첽ִ�н����
		1: �˶����Ѿ�ִ�н������ҳɹ�
		< 0: �˶����Ѿ�ִ�н�������ʧ��
	*/
	virtual int  FiniFlag() { return m_iFiniFlag; }
	void SetFiniFlag( int iVal ) { m_iFiniFlag = iVal; }

	// FiniFlag == 1ʱ����
	virtual void OnFinished( ) { }

    virtual const char* GetActionName() { return "action"; }

	/*
		ִ�ж�����ʵ�־���Ķ�������������

    	return:  1 - �����Ѿ�ִ�н�����û���첽�����Ҫ�ȴ�
                     �����Ͷ�������Ϊ��������õȴ��첽ִ�н���Ķ�������ִ��ʱӦ����0
                     ����յ��˷���ֵ�����жϰ����˶����������Ƿ��Ѿ����������û���������ִ�к�������
                 0 - �����Ѿ��������첽������Ҫ�ȴ��첽ִ�н��
                     ����յ��˷���ֵ����Ѱ����˶������������ֱ���յ��첽�����ʱ
                 < 0 - error

	*/
	virtual int Execute( Transaction* pObjTrans ) { return 0; }

	/*
		�����첽���ؽ�����ݰ�
		����յ���̨server���ص����ݰ�ʱ����parser����Ƿ�������ص����ݰ�
		����ǣ��򽻸������ڵĶ������д���

		para:
			iActionIdx �˶������������е�����(��0��ʼ)
			pResult     ��̨server���ص��첽������ݰ�
			dwResLen    ������ݰ�����
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
		0: �˶�����ûִ�л�ûִ�н������ڵȴ��첽ִ�н����
		1: �˶����Ѿ�ִ�н������ҳɹ�
		< 0: �˶����Ѿ�ִ�н�������ʧ��
	*/
	int  m_iFiniFlag;

	uint32_t m_dwTimeout; // ms
	TActionToken m_ullToken;
};


/**  �첽����ӿ�
 *   - ˵����
 *   -# �������ӿڣ��˽ӿ���Ӧ��ʵ�֣�Ӧ��ʵ�ֵ������඼����Ӵ˽ӿ�����
 *   -# �������Ƕ�����Ĺ����ߺ����������ʹ�� Transaction::GetAction ����������ȡ�þ��嶯����ִ��
 *   -# ���ʹ�� Transaction::GetAction ��ȡ����ʱ������յ�NULLָ�룬����Ϊû�и��ද��Ҫִ�У�����������
 *   -# ��������ִ�й����У����������ڲ�������Ҫ�ȴ��첽�����������ֱ���ȵ��첽�����ʱ
 *   -# �����ڹ�������У���ܻ��������Ƿ�ʱ
 *   -# ������ִ�й����У���ܻ���� Transaction::IsFinished �ж������Ƿ�ִ�н���
 *   -# ����ʱʱ����ܻ���� Transaction::OnFinished ( TRANC_ERR_TIMEOUT , ...)
 *   -# ����ɹ�����ʱ�������ڲ����������������Ҳ��Ϊ�ǳɹ�����������ܻ���� Transaction::OnFinished ( TRANC_ERR_SUCCESS , ...)
 *   -# �ɹ������ܵ����񣬿�ܻ���� IApp::OnReleaseTransaction ֪ͨӦ�û���������󣻼���ʧ��ʱ����ҪӦ���Լ��ͷ��������
 *   -# Transaction::Init �� Transaction::Fini ��ΪӦ��ʹ�õĽӿڣ������ʱ����ʹ��
 *   - Ҫ��
 *   -# ����ɹ������ܺ󣬲�Ҫ��Ӧ���м��������������ָ�룬�����ܵ��� IApp::OnReleaseTransaction �ͷ������������
 *   - ���ɣ�
 *   -# �ͷ���϶������󣬿����� TransactionFrame::OnReleaseTransaction �н���
 */
class Transaction : public IObject
{
	const static int MAX_ACTION_NUM = 64;

public:
	Transaction();

	virtual ~Transaction(){ m_iActionCount = 0; }

	/**  \brief  ��ȡ�������ƣ����������־ʱʹ��
     *   \param  ��
     *   \return ��������
     */
    virtual const char* GetTransactionName() { return "transaction"; }

	virtual void Reset();

	void SetTransactionID(TTransactionID dwTransactionID) { m_dwTransactionID = dwTransactionID; }
	TTransactionID GetTransactionID() { return m_dwTransactionID; }

	/**  \brief  ��Ӷ�����������
     *   \param  [in] pObjAction      ��������
     *   \return TActionIndex         ����Ϊ��������Ķ���ID������-1ʱ���˶������ܼ���������
     */
    TActionIndex AddAction(IAction* poAction);

	/**  \brief  ��ȡ��������ͨ�������Ż�ȡ����������������ʱ������NULL
     *   \param  [in] iIndex   ��������
     *   \return IAction*      ��������ָ�룻����NULLʱ����ʾû�и��ද��Ҫִ����
     */
    IAction* GetAction(TActionIndex iIndex);

	// ms
	unsigned int GetTimeout() { return m_dwTimeout; }
	void SetTimeout( uint32_t dwTimeout ) { m_dwTimeout = dwTimeout; }

	bool IsFinished() { return m_bFinished; }

	/**  \brief  ���֪ͨ����ĳ�����������ˣ�������������һ����ʼ�������̰���
     *   \param  [in] iIndex       ִ�н����Ķ�������
     *   \return ��
     */
    //virtual void OnActionFinish(TActionIndex iIndex){}

	/**   ���֪ͨ�������������Ѿ������ˣ��������������ս���Ĵ���
     */
    virtual void OnFinished( int iErrCode ){}

    virtual void OnActionFinished(TActionIndex iActionIdx) {}

	int GetActionCount() const
    {
        return m_iActionCount;
    }

	// ��������action��token
	void SetActionsToken();

    //����ĳ��Action��Token
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

