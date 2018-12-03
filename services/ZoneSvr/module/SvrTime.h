#pragma once
#include "singleton.h"
#include "define.h"

class SvrTime : public TSingleton<SvrTime>
{
public:
    SvrTime() {}
    ~SvrTime() {}

    bool Init();

    //获取开服时间(单位:秒)
    uint64_t GetOpenSvrTime();

    //获取开服第几天了
    int GetOpenSvrDay();

private:
    uint64_t m_ullOpenSvrTime;
};