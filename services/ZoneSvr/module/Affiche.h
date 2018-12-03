#pragma once

#include "define.h"
#include "common_proto.h"
#include "cs_proto.h"
#include "singleton.h"
#include "player/PlayerData.h"

using namespace PKGMETA;

class Affiche : public TSingleton<Affiche> 
{
public:
    Affiche() {};
    virtual ~Affiche() {};
    bool Init();
    void UpdateFromGameData();
    int GetAffiche(PlayerData* pstData);
    int SendNtf(PlayerData* pstData);
private:
    uint8_t m_bAfficheNum;
    DT_AFFICHE_INFO m_oAfficheInfo[MAX_AFFICHE_NUM];
    SCPKG m_stScPkg;
};

