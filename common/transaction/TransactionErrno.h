#ifndef _TRANSACTION_ERRNO_H_
#define _TRANSACTION_ERRNO_H_

/** 成功 */
#define     TRANC_ERR_SUCCESS    0 

/** 超时 */
#define     TRANC_ERR_TIMEOUT    -1

/** 预分配ID已用完 */
#define     TRANC_ERR_ID_USEUP   -2

/** 参数对象为空 */
#define     TRANC_ERR_OBJ_ISNULL -3

/** 组合动作已满，不能容纳更多子动作 */
#define     TRANC_ERR_COMPOSITEACTION_FULL -4

/** Action的STEP值无效 */
#define     TRANC_ERR_INVALID_STEP   -5

/** connector句柄无效 */
#define     TRANC_ERR_INVALID_HCONNECTOR -6

/** 数据(data)参数无效 */
#define     TRANC_ERR_INVALID_DATA   -7

/** 数据头(head)无效 */
#define     TRANC_ERR_INVALID_HEAD   -8

/** 数据头(head)内容错误 */
#define     TRANC_ERR_HEAD_CONTENT   -9

/** 连接当前是断开的 */
#define     TRANC_ERR_CONNECTOR_BROKEN    -10

/** 连接对象不存在 */
#define     TRANC_ERR_CONNECTOR_NOTEXIST  -11

/** 发送缓冲已满 */
#define     TRANC_ERR_SEND_BUFFER_FULL    -12

/** 发送数据失败 */
#define     TRANC_ERR_SEND_DATA_FAIL      -13

/** 本地数据格式转换为网络格式失败 */
#define     TRANC_ERR_CONVERT_HOST_TO_NET -14

/** 打包数据失败 */
#define     TRANC_ERR_PACK_DATA_FAIL      -15

/** 会话信息无效 */
#define     TRANC_ERR_INVALID_SESSION     -16

/** Action的索引值无效 */
#define     TRANC_ERR_INVALID_ACTION_INDEX   -17

/** Action的索引值不连续 */
#define     TRANC_ERR_ACTION_INDEX_DISCONTINOUS   -18

/** 暂停接收数据包 */
#define     TRANC_ERR_BREAK_RECV       -19

/** 处理接收的数据包出错 */
#define     TRANC_ERR_PROCESS_FAILED   -20

/** 指定的prototype无效 */
#define     TRANC_ERR_INVALID_PROTOTYPE   -21

/** 指定的prototype不存在 */
#define     TRANC_ERR_PROTOTYPE_NOT_EXIST -22

/** 网络连接不支持此操作 */
#define     TRANC_ERR_CONNECTOR_UNSUPORTED -23

/** 此操作不被支持 */
#define     TRANC_ERR_UNSUPORTED -24

/** 唤醒事务出错 */
#define     TRANC_ERR_WAKE_TRANSACTION_FAIL -25

/** 执行事务出错 */
#define     TRANC_ERR_EXECUTE_TRANSACTION_FAIL -26

/** 请求频率达到上限 */
#define     TRANC_ERR_MAX_REQUEST_RATE -27

#define     TRANC_ERR_ACTION_ERR -28

/**@}*/


#endif

