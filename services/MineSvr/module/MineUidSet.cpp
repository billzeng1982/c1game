

#include "MineUidSet.h"
#include "LogMacros.h"
#include "MinePlayer.h"
#include "strutil.h"

#define UIN_SPLIT 10


bool MineUidSet::Init(const char * szFileName)
{

	set<uint64_t> tmp;
	for (int i = 0; i < UIN_SPLIT; i++)
	{
		m_VectSet.push_back(tmp);
	}
	this->Clear();
	StrCpy(m_szFileName, szFileName, sizeof(m_szFileName));
	if (!InitFile())
	{
		return false;
	}
	m_ullLastSaveTime = CGameTime::Instance().GetCurrSecond();
	return true;
}

bool MineUidSet::InitFile()
{
	m_iFileUinNum = 0;
	if (0 != access(m_szFileName, 0))
	{
		m_fp = fopen(m_szFileName, "wb+");  //打开或创建文件,允许读写
		if (NULL == m_fp)
		{
			LOGERR_r("create file<%s> failed", m_szFileName);
			return false;
		}
	}
	else
	{
		m_fp = fopen(m_szFileName, "rb+");  //读写打开或建立一个二进制文件,允许读和写
		if (NULL == m_fp)
		{
			LOGERR_r("open file<%s> failed!", m_szFileName);
			return false;
		}

		if (this->ReadFile() != 0)
		{
			return false;
		}
	}
	WriteFile();
	return true;
}

void MineUidSet::Insert(uint64_t Uin)
{
	m_VectSet[(Uin >> 10) % UIN_SPLIT].insert(Uin);
}

void MineUidSet::Clear()
{
	for (int i = 0; i < (int)m_VectSet.size(); i++)
	{
		m_VectSet[i].clear();
	}
	SettleBegin();
}

void MineUidSet::Update(bool bIdle)
{
	uint64_t ullNow = CGameTime::Instance().GetCurrSecond();
	if (m_ullLastSaveTime + 300 < ullNow)
	{
		m_ullLastSaveTime = ullNow;
		WriteFile();
	}

}


void MineUidSet::Fini()
{
	WriteFile();
	Clear();
	if (m_fp)
	{
		fclose(m_fp);
	}

}

int MineUidSet::ReadFile()
{
	if (!m_fp)
	{
		LOGERR_r("readfile File not exist.");
		return -1;
	}

	fseek(m_fp, 0, SEEK_SET);

	//从文件中初始化数据
	if (1 != fread(&m_iFileUinNum, sizeof(m_iFileUinNum), 1, m_fp))
	{
		LOGERR_r("read data from file<%s> faild", m_szFileName);
		return -2;
	}
	uint64_t tmp = 0;
	for (int i = 0; i < m_iFileUinNum; i++)
	{
		if (1 != fread(&tmp, sizeof(tmp), 1, m_fp))
		{
			LOGERR_r("read data from file<%s> faild", m_szFileName);
			return -3;
		}
		this->Insert(tmp);
	}
	return 0;
}

int MineUidSet::WriteFile()
{
	if (!m_fp)
	{
		LOGERR_r("readfile File not exist.");
		return -1;
	}


	m_iFileUinNum = 0;

	fseek(m_fp, sizeof(m_iFileUinNum), SEEK_SET);
	for (int i = 0; i < UIN_SPLIT; i++)
	{
		if (!m_VectSet[i].empty())
		{
			for (set<uint64_t>::iterator it = m_VectSet[i].begin(); it != m_VectSet[i].end(); it++)
			{
				if (fwrite(&*it, sizeof(*it), 1, m_fp) != 1)
				{
					LOGERR_r("write data to file<%s> failed!", m_szFileName);
					return -4;
				}
				m_iFileUinNum++;
			}
		}
	}
	fseek(m_fp, 0, SEEK_SET);
	if (fwrite(&m_iFileUinNum, sizeof(m_iFileUinNum), 1, m_fp) != 1)
	{
		LOGERR_r("write data to file<%s> failed!", m_szFileName);
		return -4;
	}
	fflush(m_fp);
	return 0;
}

void MineUidSet::SettleBegin()
{
	m_iSettleCurVectorIndex = 0;
	m_SettleCurSetIt = m_VectSet[m_iSettleCurVectorIndex].begin();
	while (m_SettleCurSetIt == m_VectSet[m_iSettleCurVectorIndex].end() && !SettleIsEnd())
	{

		m_iSettleCurVectorIndex++;
		m_SettleCurSetIt = m_VectSet[m_iSettleCurVectorIndex].begin();
	}
}

bool MineUidSet::SettleIsEnd()
{
	return (m_iSettleCurVectorIndex + 1 == UIN_SPLIT) && m_SettleCurSetIt == m_VectSet[m_iSettleCurVectorIndex].end();
}

void MineUidSet::SettleNext()
{
	m_SettleCurSetIt++;
	if (m_VectSet[m_iSettleCurVectorIndex].end() != m_SettleCurSetIt)
	{
		return;
	}
	else
	{
		while (m_SettleCurSetIt == m_VectSet[m_iSettleCurVectorIndex].end() && !SettleIsEnd())
		{
			m_iSettleCurVectorIndex++;
			m_SettleCurSetIt = m_VectSet[m_iSettleCurVectorIndex].begin();
		}

	}
}

uint64_t MineUidSet::SettleGetCur()
{
	return *m_SettleCurSetIt;
}
