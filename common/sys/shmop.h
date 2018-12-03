#ifndef _IPC_SHM_H_
#define _IPC_SHM_H_

#include <sys/ipc.h>
#include <sys/shm.h>

/*
 * 返回值
 *  -1 : error
 *   0 : attach an existed shm
 *   1 : create a new shm
 */
int GetShm( void **ppvShm, key_t iKey, int iSize, int iFlag );

int DetachShm( const void* shmaddr );

/*
	返回值
	A valid segment identifier, shmid, is returned on success, -1 on error
*/
int GetNewShm( void **ppvShm, int iSize );


#endif

