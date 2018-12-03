#include <stdio.h>
#include "../PKGMETA/common_proto.h"
#include "../PKGMETA/cs_proto.h"
#include "../PKGMETA/ss_proto.h"

using namespace PKGMETA;


int main()
{
    //printf("sizeof(DT_REPLAY_RECORD_FILE_BODY)=%lu\n", sizeof(DT_REPLAY_RECORD_FILE_BODY));
    //printf("sizeof(DT_ROLE_TASK_INFO)=%lu\n", sizeof(DT_ROLE_TASK_INFO) );
    printf("sizeof(CSPkg)=%lu\n", sizeof(CSPkg));
    printf("sizeof(SSPKG)=%lu\n",sizeof(SSPKG));
    printf("sizeof(SCPKG)=%lu\n", sizeof(SCPKG));
    printf("sizeof(DT_ROLE_MISC_INFO)=%lu\n", sizeof(DT_ROLE_MISC_INFO));
    printf("sizeof(DT_TROOP_INFO)=%lu\n", sizeof(DT_TROOP_INFO));
    printf("sizeof(SC_PKG_MS_COMPOSITE_RSP)=%lu\n", sizeof(SC_PKG_MS_COMPOSITE_RSP));
    printf("sizeof(SC_PKG_LOTTERY_DRAW_RSP)=%lu\n", sizeof(SC_PKG_LOTTERY_DRAW_RSP));
    printf("sizeof(SC_PKG_LOTTERY_INFO_RSP)=%lu\n", sizeof(SC_PKG_LOTTERY_INFO_RSP));
    printf("sizeof(SC_PKG_AP_INC_SYN)=%lu\n", sizeof(SC_PKG_AP_INC_SYN));
    printf("sizeof(SC_PKG_PVE_SKIP_FIGHT_RSP)=%lu\n", sizeof(SC_PKG_PVE_SKIP_FIGHT_RSP));
    printf("sizeof(SC_PKG_PVE_PURCHASE_TIMES_RSP)=%lu\n", sizeof(SC_PKG_PVE_PURCHASE_TIMES_RSP));
    printf("sizeof(SC_PKG_PVE_CHAPTER_REWARD_RSP)=%lu\n", sizeof(SC_PKG_PVE_CHAPTER_REWARD_RSP));
    printf("sizeof(SC_PKG_PVE_ACTIVITY_SYN)=%lu\n", sizeof(SC_PKG_PVE_ACTIVITY_SYN));
    printf("sizeof(SC_PKG_MAJESTY_GROUP_CARD_SYN)=%lu\n", sizeof(SC_PKG_MAJESTY_GROUP_CARD_SYN));
    printf("sizeof(SC_PKG_TUTORIAL_BONUS_RSP)=%lu\n", sizeof(SC_PKG_TUTORIAL_BONUS_RSP));
    return 0;
}

