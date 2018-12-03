#include "PlayerEquipInfo.h"
#include "LogMacros.h"
#include "PlayerDataDynPool.h"

using namespace PKGMETA;

void PlayerEquipInfo:: Init(int iMaxNum)
{
    m_iCount = 0;
    m_iMaxNum = iMaxNum;
    m_bUptFlag = false;
    m_oSeqToEquipMap.clear();
}

bool PlayerEquipInfo::PackEquipInfo(DT_ROLE_EQUIP_BLOB& rstBlob, uint16_t wVersion)
{
    DT_ROLE_EQUIP_INFO stEquipInfo;
    stEquipInfo.m_iCount = m_iCount;
    m_oSeqToEquipMapIter = m_oSeqToEquipMap.begin();
    for (int i=0; m_oSeqToEquipMapIter!= m_oSeqToEquipMap.end()&&i<m_iCount; m_oSeqToEquipMapIter++, i++)
    {
        stEquipInfo.m_astData[i] = *(m_oSeqToEquipMapIter->second);
    }

    size_t ulUseSize = 0;
	int iRet = stEquipInfo.pack((char*)rstBlob.m_szData, sizeof(rstBlob.m_szData), &ulUseSize, wVersion);
	if( iRet != TdrError::TDR_NO_ERROR)
	{
		LOGERR("pack DT_ROLE_EQUIP_BLOB failed, iRet=%d, UsedSize=%lu, equip info size=%lu", iRet, ulUseSize,sizeof(DT_ROLE_EQUIP_INFO));
		return false;
	}

	rstBlob.m_iLen = (int)ulUseSize;

    return true;
}

bool PlayerEquipInfo::InitFromDB(DT_ROLE_EQUIP_BLOB& rstBlob, uint16_t wVersion)
{
    DT_ROLE_EQUIP_INFO stEquipInfo;
    size_t ulUseSize = 0;

    int iRet = stEquipInfo.unpack((char*)rstBlob.m_szData, sizeof(rstBlob.m_szData), &ulUseSize, wVersion);
	if (iRet != TdrError::TDR_NO_ERROR)
    {
		LOGERR("unpack DT_ROLE_EQUIP_BLOB failed!");
		return false;
	}

    for (int i=0; i<stEquipInfo.m_iCount; i++)
    {
        if (!this->Add(stEquipInfo.m_astData[i]))
        {
            LOGERR("Add equip failed, seq=%u", stEquipInfo.m_astData[i].m_dwSeq);
            return false;
        }
    }

    return true;
}

DT_ITEM_EQUIP* PlayerEquipInfo::Find(uint32_t dwSeq)
{
    m_oSeqToEquipMapIter = m_oSeqToEquipMap.find(dwSeq);
    if (m_oSeqToEquipMapIter == m_oSeqToEquipMap.end())
    {
        return NULL;
    }
    else
    {
        return m_oSeqToEquipMapIter->second;
    }
}

bool PlayerEquipInfo::Add(DT_ITEM_EQUIP& rstEquip)
{
    if (m_iCount>=m_iMaxNum)
    {
        return false;
    }

    m_oSeqToEquipMapIter = m_oSeqToEquipMap.find(rstEquip.m_dwSeq);
    if (m_oSeqToEquipMapIter != m_oSeqToEquipMap.end())
    {
        return false;
    }

    DT_ITEM_EQUIP* pstEquip = PlayerDataDynPool::Instance().EquipPool().Get();
    if (pstEquip==NULL)
    {
        return false;
    }

    *pstEquip = rstEquip;

    //LOGRUN("Add Equip, seq=%u, id=%u, pstEquip=%p", pstEquip->m_dwSeq, pstEquip->m_dwId, pstEquip);

    m_oSeqToEquipMap.insert(map<uint32_t, DT_ITEM_EQUIP*>::value_type(rstEquip.m_dwSeq, pstEquip));
    ++m_iCount;

    m_bUptFlag = true;

    return true;
}

bool PlayerEquipInfo::Del( uint32_t dwSeq )
{
    m_oSeqToEquipMapIter = m_oSeqToEquipMap.find(dwSeq);
    if (m_oSeqToEquipMapIter == m_oSeqToEquipMap.end())
    {
        return false;
    }

    PlayerDataDynPool::Instance().EquipPool().Release(m_oSeqToEquipMapIter->second);
    m_oSeqToEquipMap.erase(m_oSeqToEquipMapIter);
    m_iCount--;

    m_bUptFlag = true;

    return true;
}

void PlayerEquipInfo::Clear()
{
    for( m_oSeqToEquipMapIter = m_oSeqToEquipMap.begin(); m_oSeqToEquipMapIter != m_oSeqToEquipMap.end(); m_oSeqToEquipMapIter++ )
    {
        PlayerDataDynPool::Instance().EquipPool().Release(m_oSeqToEquipMapIter->second);
    }

    this->Init();
}

