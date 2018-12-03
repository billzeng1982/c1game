/*
**  @file $RCSfile: tconnapi.h,v $
**  general description of this module
**  $Id: tconnapi.h,v 1.7 2009-09-23 07:41:26 hardway Exp $
**  @author $Author: hardway $
**  @date $Date: 2009-09-23 07:41:26 $
**  @version $Revision: 1.7 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/

#ifndef TCONNAPI_H
#define TCONNAPI_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "tdr/tdr_types.h"
#include "tconnapi/tframehead.h"

#ifndef IN
#define  IN
#endif

#ifndef OUT
#define  OUT
#endif

#ifndef INOUT
#define INOUT
#endif



/** @defgroup tconnapi
* @{
*/

enum tagTconnAPIOptionName
{
      TCONNAPI_OPT_NAME_TBUS_EXCLUSIVE_CHANNELS = 1,   /*只接收connect通道的包*/
      TCONNAPI_OPT_COUNT
};






typedef  intptr_t	TCONNDHANDLE;

typedef  intptr_t	*LPTCONNDHANDLE;

extern LPTDRMETA g_pstConnapiFrameHead;

#define TCONNAPI_FRAMEHEAD_HTON(net, host, ver)		tdr_hton(g_pstConnapiFrameHead, net,host, ver)

#define TCONNAPI_FRAMEHEAD_NTOH(host,net, ver)		tdr_ntoh(g_pstConnapiFrameHead, host, net, ver)

#define TCONNAPI_MAX_BUFF		0x10000



/**
*@brief 使用tconnapi之前先必须调用该函数
*@brief 初始化API信息
*@param pszBuff[IN]  tbus share memory key,if set to 0,use 1688 by default
*@retval   =0:success
*@retval   <0:fail
*@retval  -1 初始化bus 失败,
*@retval   -2 获取TFRAMEHEAD Meta失败
*/
int tconnapi_init(IN int iKey);



/**
*@brief TFRAMEHEAD 解包
*@param pszBuff[IN]  		解包目的缓冲区
*@param iBuff[IN]  		目的缓冲区长度
*@param pstHead[INOUT]	TFRAMEHEAD结构地址
*@param piHeadLen[OUT]	解包后网络长度
*@retval   =0:success
*@retval   <0:解包失败
*@retval   =-1 输入参数非法
*@retval   =-2 tdr解包失败
*@see
*/
int tconnapi_decode(IN const char* pszBuff,IN  int iBuff,OUT TFRAMEHEAD* pstHead,OUT int* piHeadLen);
int tconnapi_wap_decode(IN const char *pszBuff, IN int iBuff, OUT char *pPkgBuff, INOUT int *piPkgLen);


/**
*@brief FrameHead 打包
*@param pszBuff[IN]  		打包目的缓冲区
*@param piBuff[INOUT]  	IN:目的缓冲区长度
*                                         OUT:打包网络长度
*@param pstHead[IN]		FrameHead地址
*@retval   =0:success
*@retval   <0:打包失败
*@retval  =-1 输入参数非法
*@retval   =-2 tdr 打包失败
*@see
*/
int tconnapi_encode(IN char* pszBuff,INOUT  int* piBuff, IN TFRAMEHEAD* pstHead);
int tconnapi_wap_encode(OUT char *pszBuff, INOUT int *piBuff, IN const char *pPkgBuff, IN int iPkgLen);


/**
*@brief  释放API信息,对象生命周期结束必须调用该API释放
*/
void tconnapi_fini(void);



/**
*@brief 创建API句柄收发消息
*@param iProcID[IN]  bus ID
*@param ppstHandle[OUT]  句柄指针
*@retval     =0:success
*@retval    <0:fail
*@retval      = -1  分配句柄内存失败
*@retval      = -2  创建BUS 句柄失败
*@retval      = -3  绑定BUS 通道失败
*/
int tconnapi_create(IN int iProcID, OUT TCONNDHANDLE *pstHandle);



/**
*@brief 释放句柄
*@param pstHandle[IN]  句柄指针
*@param iDst[IN] 		   对端地址
*@retval    =0 success
*@retval    <0 fail
*@retval    =-1 连接对端BUS 通道失败
*/
int tconnapi_connect(IN TCONNDHANDLE iHandle,IN int iDst);

/**
*@brief   注意使用该接口收取包长最大不超过65536字节
*@brief  从bus中收包
*@param pstHandle[IN]  	句柄指针
*@param piSrc[INOUT]  	bus源地址
*@param pszBuff[OUT]  	应用消息体缓冲区起始地址
*@param piBuff[INOUT]       IN  :缓冲区长度
*						OUT:消息体长度
*@param pstHead[OUT]  	解包的FrameHead
*@retval     =0  收到包
*@retval    <0 没有收到包,See errno
*@retval    -1 Tbus通道没有包
*@retval    -2 输入参数非法
*@retval    -3 TFrameHead 解包错
*@retval    -4 应用缓冲区不够
*/
int tconnapi_recv(IN TCONNDHANDLE iHandle, INOUT int *piSrc,OUT char* pszBuff,INOUT int* piBuff,OUT TFRAMEHEAD* pstHead);


/**
*@brief  往bus中发包
*@param iHandle[IN]  		句柄指针
*@param iDst[IN]  			bus目的地址
*@param pszBuff[IN]  		应用消息体缓冲区起始地址
*@param iBuff[IN  ]  	       缓冲区长度
*@param pstHead[IN]  		打包的FrameHead,若为空则发送句柄上次收到的包
*@retval    =0 send package success
*@retval    <0 发送失败
*@retval     = -1 TFRAMEHEAD打包出错
*@retval     = -2 Tbus 发送失败
*/
int tconnapi_send(IN TCONNDHANDLE iHandle, IN  int iDst, IN char* pszBuff,IN  int iBuff,IN TFRAMEHEAD* pstHead);



/**
*@brief 释放API句柄
*@param ppstHandle[OUT]  句柄指针
*@retval
*/
void tconnapi_free(IN TCONNDHANDLE * piHandle);



/**
*@brief 设置bus句柄属性
*@param iHandle  				句柄指针
*@param iOptionName   		属性名
*@param pvOptionValue               属性值,参考tagTconnAPIOptionName定义
*@param dwOptionLen                  属性长度
*@retval      =0  success
*@retval      <0 fail
*@retval     =-1 设置bus句柄属性定义
*/
int tconnapi_set_handle_opt(IN TCONNDHANDLE iHandle,IN int iOptionName,IN const void *pvOptionValue,IN unsigned int dwOptionLen);




/**
*@brief  获得tconnd 关闭连接提示信息
*@param iReason  关闭原因
*@retval   返回连接关闭提示信息
*/
const char* tconnapi_get_closestring(IN int iReason);


/**
*@brief  获得tconnd 构建版本
*@retval   返回版本构建信息
*/
const char* tconnapi_get_version(void);


/**
*@brief  内部接口
*/
int tconnapi_initialize(const char *a_pszGCIMKey, int a_iBusinessid);


/** @} */

#ifdef __cplusplus
}
#endif


#endif /* TCONNAPI_H */
