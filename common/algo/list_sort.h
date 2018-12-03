#ifndef _LIST_SORT_2011_02_25
#define _LIST_SORT_2011_02_25

#include "list.h"

/**
 * 双向链表插入排序, in ascending order 
 * @head: 待排序链表表头，表示整个链表
 * @compare: compare回调函数，在该函数内计算链表的宿主结构并对宿主的key进行比较，0: equal, -1: less, 1: greater
 */
void list_insert_sort( struct list_head* head, 
					   int(*compare)( const struct list_head* , const struct list_head* ) );

#endif

