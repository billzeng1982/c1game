#include <stdio.h>
#include "shmop.h"

static void* _GetShm( key_t iKey, int iSize, int iFlag )
{
    int iShmID  = 0;
    void* pvShm  = NULL;
    char sErrMsg[50];

	if ((iShmID = shmget(iKey, iSize, iFlag)) < 0) 
    {
		snprintf(sErrMsg, sizeof(sErrMsg), "shmget %d %d", iKey, iSize);
		perror(sErrMsg);
		return NULL;
	}

    if ( (pvShm = shmat(iShmID, NULL ,0)) == (void *) -1 )
    {
		perror("shmat");
		return NULL;
	}
    
	return (char*)pvShm;
}


/*
 * 返回值
 *  -1 : error
 *   0 : attach an existed shm
 *   1 : create a new shm
 */
int GetShm( void** ppvShm, key_t iKey, int iSize, int iFlag )
{
    void* pvShm;

	if ( !( pvShm = _GetShm( iKey, iSize, iFlag & (~IPC_CREAT) ) ) ) 
    {
		if (!(iFlag & IPC_CREAT))
        {
            return -1;
        }

        if ( !( pvShm = _GetShm(iKey, iSize, iFlag) ) )
        {
            return -1;
        }

        // 注意新创建的shm不做清0操作，由调用者负责初始化
		*ppvShm = pvShm;
		return 1;
	}
    
	*ppvShm = pvShm;

    return 0;
}

int DetachShm( const void* shmaddr )
{
    return shmdt( shmaddr );
}


/*
	返回值
	A valid segment identifier, shmid, is returned on success, -1 on error
*/
int GetNewShm( void **ppvShm, int iSize )
{
    int iShmID  = 0;
    void* pvShm  = NULL;
    char sErrMsg[50];

	if (( iShmID = shmget(IPC_PRIVATE, iSize, 0666 )) < 0) 
    {
		snprintf(sErrMsg, sizeof(sErrMsg), "shmget %d", iSize);
		perror(sErrMsg);
		return -1;
	}

    if ( (pvShm = shmat(iShmID, NULL ,0)) == (void *) -1 )
    {
        printf("attach to shm %d failed\n", iShmID );
		perror("shmat ");
		return -1;
	}
    
	*ppvShm = pvShm;

    return iShmID;
}


