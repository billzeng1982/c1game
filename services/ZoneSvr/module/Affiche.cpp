#include "Affiche.h"
#include "../gamedata/GameDataMgr.h"
#include "../framework/ZoneSvrMsgLayer.h"

bool Affiche::Init()
{
    UpdateFromGameData();
    return true;
}

void Affiche::UpdateFromGameData()
{
    RESAFFICHE* pstResAffiche = NULL;// CGameDataMgr::Instance().GetResAfficheMgr().Find(PURCHASE_ID_AP);
    int iResNum = CGameDataMgr::Instance().GetResAfficheMgr().GetResNum();
    m_bAfficheNum = 0;
    for (int i = 0; i < iResNum && m_bAfficheNum < MAX_AFFICHE_NUM; i++ )
    {
        pstResAffiche = CGameDataMgr::Instance().GetResAfficheMgr().GetResByPos(i);
        if (NULL == pstResAffiche)
        {
            LOGERR("ResAffiche is null, please check the Pos<%u>!", i);
            continue;
        }
        m_oAfficheInfo[m_bAfficheNum].m_dwId = pstResAffiche->m_dwId;
        StrCpy(m_oAfficheInfo[m_bAfficheNum].m_szTitle, pstResAffiche->m_szTitle, MAX_AFFICHE_TITLE_LEN);
        StrCpy(m_oAfficheInfo[m_bAfficheNum].m_szContent, pstResAffiche->m_szContent, MAX_AFFICHE_CONTENT_LEN);
        m_oAfficheInfo[m_bAfficheNum].m_bDisplay = pstResAffiche->m_bDisplay;
        m_bAfficheNum++;
    }
}

int Affiche::GetAffiche(PlayerData* pstData)
{
    return SendNtf(pstData);

}

int Affiche::SendNtf(PlayerData* pstData)
{
    SC_PKG_AFFICHE_GET_NTF& rstNtf = m_stScPkg.m_stBody.m_stAfficheGetNtf;
    rstNtf.m_bCount = 0;
    for (int i = 0 ;i < m_bAfficheNum; i++)
    {
        if (0 == m_oAfficheInfo[i].m_bDisplay)
        {
            continue;
        }
        memcpy(&rstNtf.m_astAfficheList[rstNtf.m_bCount], &m_oAfficheInfo[i], sizeof(DT_AFFICHE_INFO));
        rstNtf.m_bCount++;
    }
    m_stScPkg.m_stBody.m_stAfficheGetNtf.m_nErrNo = ERR_NONE;
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_AFFICHE_GET_NTF;
    ZoneSvrMsgLayer::Instance().SendToClient(pstData->m_pOwner, &m_stScPkg);
    return 0;
}
