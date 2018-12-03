#ifndef _LIST_I_H_
#define _LIST_I_H_

/*
	���ڵ�ַ+ƫ��(index)��˫��ѭ������, �ɴ��ڹ����ڴ�
	��Ƕʽ����,˼��ͬlist.h
*/

#include "container_of.h"
#include "comm/tlist.h"


// list_move - delete from one list and add as another's head
#define TLIST_MOVE(pstList, pstNode) do { \
	TLIST_DEL( pstNode ); \
	TLIST_INSERT_NEXT( pstList, pstNode ); \
}while(0)

// list_move_tail - delete from one list and add as another's tail
#define TLIST_MOVE_TAIL( pstList, pstNode ) do { \
	TLIST_DEL(pstNode); \
	TLIST_INSERT_PREV( pstList, pstNode ); \
}while(0)

#define TLIST_ENTRY( ptr, type, member ) container_of( ptr, type, member )

#endif

