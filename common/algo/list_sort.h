#ifndef _LIST_SORT_2011_02_25
#define _LIST_SORT_2011_02_25

#include "list.h"

/**
 * ˫�������������, in ascending order 
 * @head: �����������ͷ����ʾ��������
 * @compare: compare�ص��������ڸú����ڼ�������������ṹ����������key���бȽϣ�0: equal, -1: less, 1: greater
 */
void list_insert_sort( struct list_head* head, 
					   int(*compare)( const struct list_head* , const struct list_head* ) );

#endif

