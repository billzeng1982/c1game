#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include "CpuSampleStats.h"
#include "og_comm.h"
using namespace std;
void CpuSampleStats::BeginSample(const char* pszKey,  ...)
{
    if (!m_bCpuSampleSwitch)
    {
        return;
    }

    SampleItem* pstSampleItem = NULL;
    do 
    {
        if (pszKey == NULL)
        {

            break;
        }
        va_list	ap;
        va_start(ap, pszKey);
        vsnprintf(m_szKeyBuff, sizeof(m_szKeyBuff), pszKey, ap);
        va_end(ap);
        string key(m_szKeyBuff);
        SampleDict_t::iterator iter = m_SampleItemDict.find(key);
        
        if (iter == m_SampleItemDict.end())
        {
            pair<SampleDict_t::iterator, bool> Ret = m_SampleItemDict.insert(make_pair(key, SampleItem()));
            if (Ret.second)
            {
                pstSampleItem = &Ret.first->second;
                pstSampleItem->m_sKey = key;
            }
        }
        else
        {
            pstSampleItem = &iter->second;
        }
        if (pstSampleItem == NULL)
        {
            break;
        }
        //pstSampleItem->m_ulBeginTicks = clock();
        gettimeofday(&pstSampleItem->m_stBeginTv, NULL);

    } while (0);
    m_beginStack.push(pstSampleItem);   //空也进栈,保持一致
}



void CpuSampleStats::EndSample()
{
    if (!m_bCpuSampleSwitch)
    {
        return;
    }
    SampleItem* pstSampleItem = m_beginStack.top();
    if (pstSampleItem == NULL)
    {
        assert(false);
        m_beginStack.pop(); //空也出栈,保持一致
        return;
    }
    pstSampleItem->m_luCalls++;
    //uint64_t lElapsedTicks = clock() - pstSampleItem->m_ulBeginTicks; 
    gettimeofday(&m_stCurrTv, NULL);
    uint64_t fElapsedUs = UsPass(&m_stCurrTv, &pstSampleItem->m_stBeginTv);
    //double fElapsedMs = (double)lElapsedTicks * 1000 * 1000 / CLOCKS_PER_SEC;
    
    pstSampleItem->m_fTotalCost += fElapsedUs;

    if (fElapsedUs < pstSampleItem->m_fMinCost)
    {
        pstSampleItem->m_fMinCost = fElapsedUs;
    }

    if (fElapsedUs > pstSampleItem->m_fMaxCost)
    {
        pstSampleItem->m_fMaxCost = fElapsedUs;
    }
    m_beginStack.pop();
}

void CpuSampleStats::Dump()
{
    
    SampleItem* pstSampleItem = NULL;
    FILE * pFile = fopen("CpuSample.log", "w");
    if (pFile == NULL)
    {
        assert(false);
        return;
    }
    fprintf(pFile, "FunctionName|CalledTimes|AvgCpuCost|MaxCpuCost|MinCpuCost\n");
    float fAvgTimeCost = 0.0f;
    for (SampleDict_t::iterator iter = m_SampleItemDict.begin(); iter != m_SampleItemDict.end(); iter++)
    {
        pstSampleItem = &iter->second;
        if (pstSampleItem->m_luCalls > 0)
        {
            fAvgTimeCost = pstSampleItem->m_fTotalCost / pstSampleItem->m_luCalls;
        }
        fprintf(pFile, "%s|%lu|%lu|%lu|%lu\n", pstSampleItem->m_sKey.c_str(), pstSampleItem->m_luCalls,
            (uint64_t)fAvgTimeCost, (uint64_t)pstSampleItem->m_fMaxCost, (uint64_t)pstSampleItem->m_fMinCost);
    }
    fclose(pFile);
}

