#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include "tbus/tbus.h"
#include <string.h>
#include "CommBusLayer.h"
#include "common_proto.h"
#include "ss_proto.h"

/*
usage: ./send GCIMShmKey srcAddr dstAddr
*/
using namespace PKGMETA;
using namespace std;

int GetToken(vector<string>& Tokens)
{
    ifstream ifs("token.txt");
    if (!ifs)
    {
        printf("open file error!");
        return -1;
    }
    string s;
    while (getline(ifs, s))
    {
        Tokens.push_back(s);
    }
    printf("read token count=%d", (int)Tokens.size());
    return 0;
}

int main(int argc, char** argv)
{
    vector<string> Tokens;
    if (0 != GetToken(Tokens))
    {
        return -11;
    }
    CCommBusLayer oBusLayer;
    int iGCIMKey = 30000;
    int iOnceCount = atoi(argv[1]);
    char sSrcAddr[] = "0.0.2.1";
    char sDstAddr[] = "0.0.16.1";
    int iSrcAddr = 0;
    inet_aton(sSrcAddr, (struct in_addr*)&iSrcAddr);
    int iDstAddr = 0;
    inet_aton(sDstAddr, (struct in_addr*)&iDstAddr);

    if (oBusLayer.Init(iGCIMKey, iSrcAddr) != 0)
    {
        printf("init failed!\n");
        return -2;
    }
    SSPKG m_stSsPkg;
    bzero(&m_stSsPkg, sizeof(m_stSsPkg));
    char buf[ sizeof(SSPKG) + 5];
    size_t PackLen = 0;
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_SDK_ACCOUNT_LOGIN_REQ;
    m_stSsPkg.m_stHead.m_ullReservId = 1;
    SS_PKG_SDK_ACCOUNT_LOGIN_REQ& rstSDKLoginReq = m_stSsPkg.m_stBody.m_stSDKAccountLoginReq;
    int iCount = 0;
    int iTokenCnt = Tokens.size();
    do 
    {
        while (++iCount % iOnceCount)
        {
            //StrCpy(rstSDKLoginReq.m_szToken, 1, PKGMETA::MAX_LEN_SDK_TOKEN_PARA);
            if (iCount % 50)
            {
            }
            snprintf(rstSDKLoginReq.m_szToken, MAX_LEN_SDK_TOKEN_PARA, "%s", Tokens[iCount%iTokenCnt].c_str());
            strncpy(rstSDKLoginReq.m_szChannelName, "XY", PKGMETA::MAX_NAME_LENGTH);
            strncpy(rstSDKLoginReq.m_szOpenID, "OpenId", PKGMETA::MAX_NAME_LENGTH);
            TdrError::ErrorType iRet = m_stSsPkg.pack(buf, sizeof(buf), &PackLen);
            if (iRet != TdrError::TDR_NO_ERROR)
            {
                printf("pack ss pkg error! <%d> \n", iRet);
                return -1;
            }
            oBusLayer.Send(iDstAddr, buf, PackLen);
        }
        printf("send msg count total = %d", iCount);
    } while (getchar() != 'q');


    return 0;
}