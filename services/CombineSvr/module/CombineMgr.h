#pragma once
#include <vector>
#include "singleton.h"
#include "../thread/MysqlWorkThread.h"
#include "../cfg/CombineSvrCfgDesc.h"

using namespace std;
class CombineMgr : public TSingleton<CombineMgr>
{
public:


public:
	bool Init(COMBINESVRCFG* m_pstConfig);
	//由线程调用,线程只会修改自己的状态, 不用锁
	void ChangeState(int iIndex, int iResult);
	void Fini();
	void Update();
// 	int Combine();
// 	void Work();
private:
	vector<CMysqlWorkThread*> m_astMysqlWorkThreads;
	vector<int>	m_CombineState;
	COMBINESVRCFG* m_pstConfig;
	int m_iMysqlWorkThreadNum;

};