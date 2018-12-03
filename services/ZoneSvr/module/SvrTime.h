#pragma once
#include "singleton.h"
#include "define.h"

class SvrTime : public TSingleton<SvrTime>
{
public:
    SvrTime() {}
    ~SvrTime() {}

    bool Init();

    //��ȡ����ʱ��(��λ:��)
    uint64_t GetOpenSvrTime();

    //��ȡ�����ڼ�����
    int GetOpenSvrDay();

private:
    uint64_t m_ullOpenSvrTime;
};