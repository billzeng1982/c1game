#include "Replay.h"

using namespace PKGMETA;

Replay::Replay()
{
    m_ullLastUpdateTime = 0;
    bzero(&m_stReplayList, sizeof(m_stReplayList));
}

Replay::~Replay()
{

}


void Replay::UpdateReplay(SS_PKG_REFRESH_REPLAYLIST_NTF& rstSsPkgBodyRsp)
{
    if (rstSsPkgBodyRsp.m_ullTimeStamp == m_ullLastUpdateTime)
    {
        return;
    }

    memcpy(&m_stReplayList, &rstSsPkgBodyRsp.m_stReplayList, sizeof(m_stReplayList));
    m_ullLastUpdateTime = rstSsPkgBodyRsp.m_ullTimeStamp;
    return;
}




void Replay::GetReplayList(CS_PKG_REFRESH_REPLAYLIST_REQ& rstCsPkgBodyReq, SC_PKG_REFRESH_REPLAYLIST_RSP& rstScPkgBodyRsp)
{
    //�ͻ����Ѿ������µ�¼���б�����Ҫ����
    if (rstCsPkgBodyReq.m_ullTimeStamp == m_ullLastUpdateTime)
    {
        rstScPkgBodyRsp.m_nErrNo = ERR_ALREADY_LATEST;
        return;
    }

    if (rstCsPkgBodyReq.m_bGrade >= MAX_NUM_GRADE)
    {
        rstScPkgBodyRsp.m_nErrNo = ERR_OUT_OF_BOUND;
        return;
    }

    if (rstCsPkgBodyReq.m_bPage >= MAX_NUM_REPLAY/MAX_NUM_ONE_PAGE_REPLAY)
    {
        rstScPkgBodyRsp.m_nErrNo = ERR_OUT_OF_BOUND;
        return;
    }

    //���¿ͻ���¼���б�
    rstScPkgBodyRsp.m_nErrNo = ERR_NONE;
    rstScPkgBodyRsp.m_ullTimeStamp = m_ullLastUpdateTime;

    DT_ONE_GRADE_REPLAY_LIST& rstOneGradeReplayList = m_stReplayList.m_astWholeReplay[rstCsPkgBodyReq.m_bGrade];
    int iGradeCount = rstOneGradeReplayList.m_wReplayCount;

    int iPageCount = iGradeCount - (int)(MAX_NUM_GRADE * rstCsPkgBodyReq.m_bPage);
    if (iPageCount < 0)
    {
        iPageCount = 0;
    }
    else if (iPageCount > MAX_NUM_ONE_PAGE_REPLAY)
    {
        iPageCount = MAX_NUM_ONE_PAGE_REPLAY;
    }

    rstScPkgBodyRsp.m_stReplayList.m_wReplayCount = iPageCount;

    memcpy(rstScPkgBodyRsp.m_stReplayList.m_astReplayList,
            &rstOneGradeReplayList.m_astReplayList[MAX_NUM_GRADE * rstCsPkgBodyReq.m_bPage], sizeof(DT_REPLAY_INFO)*iPageCount);

    return;
}


