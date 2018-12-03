/************************************************************
*  File Module:    tconnd_routing_plugin.h
*  File Description: 
*   定义路由扩展接口
*  File History:
*  <author>  <time>            <version >   <desc>
*  jiaweixu    2011/08/26         1.0           create this moudle
*  alancxu     2012/07/09         1.1           add function interface 'tconnd_router_getAllRoutingInfo'
***********************************************************/
#ifndef _TCONND_ROUTING_PLUGIN_H
#define _TCONND_ROUTING_PLUGIN_H

#ifndef IN
#define  IN
#endif

#ifndef OUT
#define  OUT
#endif

#ifndef INOUT
#define  INOUT
#endif

#define MAX_ROUTING_ERR_MSG_LEN         1024

typedef struct tagTRoutingCtx TROUTINGCTX;
typedef struct tagTRoutingCtx *LPTROUTINGCTX;

typedef struct tagTSerializerInfo TSERIALIZERINFO;
typedef struct tagTSerializerInfo *LPTSERIALIZERINFO;

typedef struct tagTPluginInfo TPLUGININFO;
typedef struct tagTPluginInfo *LPTPLUGININFO;

typedef struct tagTRoutingInfo TROUTINGINFO;
typedef struct tagTRoutingInfo *LPTROUTINGINFO;

/*Base class of tconnd's routing plugin*/
class TRouterBase
{
public:
    virtual ~TRouterBase(){}
    /**
        * @brief Initializing routing configuration. This is called when tconnd is initialized.
        * @param pszPluginInfo[IN] Some information tconnd keeps.
        * @retval Returns 0 for success, otherwise for failure.
       */
    virtual int Init(IN const LPTPLUGININFO pszPluginInfo) = 0;
    /**
        * @brief Reloading routing configuration. This is called when tconnd is reloading.
        * @retval Returns 0 for success, otherwise for failure.
       */
    virtual int Reload() = 0;
    /**
        * @brief Acquiring routing list by some information(e.g. tframe head command, packet and so on). This is called before tconnd sends packet to logic server.
        * @param pstCtx[IN] Routing context
        * @param pstRoutingInfo[OUT] Routing information.
        * @retval Returns 0 for success, otherwise for failure.
       */
    virtual int GetRoutingList(IN const LPTROUTINGCTX pstCtx, OUT LPTROUTINGINFO pstRoutingInfo) = 0;
    /**
        * @brief Registering routing information which come from logic server into routing table. This is called when tconnd recieves TFRAMEHEAD_CMD_REGISTER_ROUTING message from logic server.
        * @param iPeerAddr[IN] Logic server's address which sends TFRAMEHEAD_CMD_REGISTER_ROUTING message. See tframe head protocol about this message.
        * @param pszRoutingArgs[IN] Routing arguments. According to these, tconnd generates routing tables.
        * @param iArgsLen[IN] The length of routing arguments.
        * @retval Returns 0 for success, otherwise for failure.
       */
    virtual int RegisterRoutingInfo(IN int iPeerAddr, IN const char *pszRoutingArgs, IN int iArgsLen) = 0;
};

struct tagTRoutingCtx
{
    /*tconnd index*/
    int iConnIdx;
    /*TFrame head command, see tframe head protocol for detail.*/
    int iFrameCmd;
    /*The size of up-link packet.*/
    int iDataLen;
    /*The up-link packet. The application can use it to implement routing based on level-7.*/
    const char *pszData;
    /*qq number, which only is valid for qq and rudp mode*/
    uint32_t dwUin;
    /*client ip*/
    uint32_t ulIp;
};

/*The status of a serializer*/
struct tagTSerializerInfo
{
    int iSerIdx; /*the serializer's index, read only*/
    int iIsPeerActive; /*the logic server's status: 1 specify active, otherwiese dead. read only*/
    int iPeerAddr; /*the logic server's address*/ 
};

/*This is used by routing plugin, which keeps tconnd's information*/
struct tagTPluginInfo
{
    unsigned int dwMaxFD;  /*max connection #*/
    int iLocalAddr; /*tconnd's address*/
    const char *pszConfPath; /*the path of routing configuration file. it may be null.*/
    int iSerCount;    /*the number of all serializers*/
    LPTSERIALIZERINFO pstSerInfoList; /*the status of all serializers*/
    void *pstTdrMeta; /*the meta data of up-link package. it is used to unpack up-link package. it may be null.*/
};

/*Routing infomation, which is returned by routing plugin. According to this, 
 * tconnd will route message to these serializers.
 */
struct tagTRoutingInfo
{
    /*The number of serializers which the tconnd will send message to. It shuold be less than or equal to tagTPluginInfo.iSerCount.*/
    int iSerCount; 
    /*A array of the index of serializers and its size is equal to tagTPluginInfo.iSerCount. Its memory is allocated by the tconnd.*/
    int *serIndexList; 
    /*routing error message*/
    char szErrMsg[MAX_ROUTING_ERR_MSG_LEN];
};


#ifdef __cplusplus
extern "C"
{
#endif

typedef TRouterBase* (*PFNTCONND_ROUTER_CREATE)(void);
typedef void (*PFNTCONND_ROUTER_DESTROY)(TRouterBase* router);
typedef int (*PFNTCONND_ROUTER_GetAllRoutingInfo)(IN TRouterBase* router, INOUT char *pszRoutingInfo, INOUT int *iRoutingInfoLen);


/*Create concrete router object*/
TRouterBase* tconnd_router_create(void);

/*Destroy the router object*/
void tconnd_router_destroy(TRouterBase* router);

/**
  *@brief Get all routing infomation, not must be realized
  *@param router[IN] the pointer which pointers to router object
  *@param pszRoutingInfo[INOUT] the buffer which stores all routing infomation
  *      - IN pszRoutingInfo the first address which starts store routing infomation
  *      - OUT pszRoutingInfo the first address which starts store routing infomation
  *@param iRoutingInfoLen[INOUT] the pointer which pointers to value is the length of storing all routing infomation buffer
  *      - IN iRoutingInfoLen the maximize length of storing all routing infomation buffer
  *      - OUT iRoutingInfoLen the used length of storing all routing infomation, used length not greater than maximize length
  *
  *@pre \e router not NULL
  *@pre \e pszRoutingInfo not NULL
  *@pre \e iRoutingInfoLen not NULL and pointer to value is greater than 0
  *
  *@retval 0 success
  *@retval other fail
 */
int tconnd_router_getAllRoutingInfo(IN TRouterBase* router, INOUT char *pszRoutingInfo, INOUT int *iRoutingInfoLen);

#ifdef __cplusplus
}
#endif

#endif

