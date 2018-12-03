#pragma once

#include <string>
#include <stack>
#include <map>
#include <time.h>
#include <set>
#include <inttypes.h>
#include <stdarg.h>
#include <sys/time.h>
#include "singleton.h"


/*
* 采样单帧某函数或程序片段执行时间
* 输出调用次数和平均耗时(ms)
*
* 注意:
* BeginSample和EndSample必须成对使用，且中间不能有return语句，否则统计会出错
* Begin和End之间可能嵌套，使用栈来管理
*/

class CpuSampleStats : public TSingleton<CpuSampleStats>
{
public:
    class SampleItem
    {
    public:
         std::string    m_sKey;
         uint64_t       m_luCalls;
         float          m_fTotalCost;   // us
         float          m_fMaxCost;     // us
         float          m_fMinCost;     // us
         //uint64_t       m_ulBeginTicks; // 执行时钟数   CLOCKS_PER_SEC 
         struct timeval m_stBeginTv;
    public:
        SampleItem()
        {
            m_sKey.clear();
            m_luCalls = 0;
            m_fTotalCost = 0.0f;
            m_fMinCost = 99000000.0f;
            m_fMaxCost = 0.0f;
            m_stBeginTv.tv_sec = 0;
            m_stBeginTv.tv_usec = 0;
            //m_ulBeginTicks = 0;
           
        }
    };

public:
    CpuSampleStats() 
    {
        m_bCpuSampleSwitch = false; 
        m_szKeyBuff[0] = '\0';
    }
    ~CpuSampleStats()
    {
        m_SampleItemDict.clear();
        while (!m_beginStack.empty())
        {
            m_beginStack.pop();
        }
    }
    void BeginSample(const char* pszKey , ...);
    void EndSample();
    void Dump();
    void OpenCpuSample() { m_bCpuSampleSwitch = true; }
//     bool GetSwitch() { return m_bCpuSampleSwitch; }
public:
    bool m_bCpuSampleSwitch;
private:
    typedef std::map<std::string, SampleItem> SampleDict_t;
    std::stack<SampleItem*> m_beginStack;
    SampleDict_t m_SampleItemDict;
    struct timeval m_stCurrTv;
    SampleDict_t::iterator m_iter;
    char m_szKeyBuff[1024];
};



