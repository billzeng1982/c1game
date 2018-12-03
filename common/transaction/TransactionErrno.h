#ifndef _TRANSACTION_ERRNO_H_
#define _TRANSACTION_ERRNO_H_

/** �ɹ� */
#define     TRANC_ERR_SUCCESS    0 

/** ��ʱ */
#define     TRANC_ERR_TIMEOUT    -1

/** Ԥ����ID������ */
#define     TRANC_ERR_ID_USEUP   -2

/** ��������Ϊ�� */
#define     TRANC_ERR_OBJ_ISNULL -3

/** ��϶����������������ɸ����Ӷ��� */
#define     TRANC_ERR_COMPOSITEACTION_FULL -4

/** Action��STEPֵ��Ч */
#define     TRANC_ERR_INVALID_STEP   -5

/** connector�����Ч */
#define     TRANC_ERR_INVALID_HCONNECTOR -6

/** ����(data)������Ч */
#define     TRANC_ERR_INVALID_DATA   -7

/** ����ͷ(head)��Ч */
#define     TRANC_ERR_INVALID_HEAD   -8

/** ����ͷ(head)���ݴ��� */
#define     TRANC_ERR_HEAD_CONTENT   -9

/** ���ӵ�ǰ�ǶϿ��� */
#define     TRANC_ERR_CONNECTOR_BROKEN    -10

/** ���Ӷ��󲻴��� */
#define     TRANC_ERR_CONNECTOR_NOTEXIST  -11

/** ���ͻ������� */
#define     TRANC_ERR_SEND_BUFFER_FULL    -12

/** ��������ʧ�� */
#define     TRANC_ERR_SEND_DATA_FAIL      -13

/** �������ݸ�ʽת��Ϊ�����ʽʧ�� */
#define     TRANC_ERR_CONVERT_HOST_TO_NET -14

/** �������ʧ�� */
#define     TRANC_ERR_PACK_DATA_FAIL      -15

/** �Ự��Ϣ��Ч */
#define     TRANC_ERR_INVALID_SESSION     -16

/** Action������ֵ��Ч */
#define     TRANC_ERR_INVALID_ACTION_INDEX   -17

/** Action������ֵ������ */
#define     TRANC_ERR_ACTION_INDEX_DISCONTINOUS   -18

/** ��ͣ�������ݰ� */
#define     TRANC_ERR_BREAK_RECV       -19

/** ������յ����ݰ����� */
#define     TRANC_ERR_PROCESS_FAILED   -20

/** ָ����prototype��Ч */
#define     TRANC_ERR_INVALID_PROTOTYPE   -21

/** ָ����prototype������ */
#define     TRANC_ERR_PROTOTYPE_NOT_EXIST -22

/** �������Ӳ�֧�ִ˲��� */
#define     TRANC_ERR_CONNECTOR_UNSUPORTED -23

/** �˲�������֧�� */
#define     TRANC_ERR_UNSUPORTED -24

/** ����������� */
#define     TRANC_ERR_WAKE_TRANSACTION_FAIL -25

/** ִ��������� */
#define     TRANC_ERR_EXECUTE_TRANSACTION_FAIL -26

/** ����Ƶ�ʴﵽ���� */
#define     TRANC_ERR_MAX_REQUEST_RATE -27

#define     TRANC_ERR_ACTION_ERR -28

/**@}*/


#endif

