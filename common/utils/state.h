#pragma once

/*
	state ����

	"�¼�"��Ӧ״̬ת��ͼ�������
*/

template<typename TContext>
class IState
{
public:
	IState(){}
	virtual ~IState() {}

	// this will execute when the state is entered
	virtual void Enter(TContext*) = 0;

	// this is the states normal update function
	virtual void Update(TContext*, int iDeltaTime) = 0;

	// this will execute when the state is exited
	virtual void Exit(TContext*) = 0;

	virtual void ChangeState(TContext*, int iNewState) = 0;

	virtual void HandleEvent(TContext*, int iEventID, void* pvEventPara) {};
};

