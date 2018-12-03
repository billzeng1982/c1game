#pragma once
#include <stdio.h>
#include <set>
#include <vector>
#include "define.h"
using namespace std;

class MineUidSet
{
public:

	bool Init(const char * szFileName);

	bool InitFile();

	void Insert(uint64_t Uin);

	void Clear();

	void Update(bool bIdle);

	void Fini();

	int ReadFile();
	int WriteFile();

	void SettleBegin();

	bool SettleIsEnd();

	void SettleNext();

	int GetUidNum() { return m_iFileUinNum; };

	//	Get前必须要判断End
	uint64_t SettleGetCur();
private:
	vector< set<uint64_t> > m_VectSet;
	char m_szFileName[255];
	FILE* m_fp;
	int m_iFileUinNum;
	uint64_t m_ullLastSaveTime;
	set<uint64_t>::iterator m_SettleCurSetIt;
	int m_iSettleCurVectorIndex;
};


