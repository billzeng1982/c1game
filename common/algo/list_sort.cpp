#include <assert.h>

#include "list_sort.h"


/**
 * @begin: 待排序链表的开始点
 * @end: 待排序链表的结束点
 * @compare: compare回调函数，在该函数内计算链表的宿主结构并对宿主的key进行比较，0: equal, <0: less, >0: greater
 */
static void __list_insert_sort( struct list_head* begin, 
					   struct list_head* end, 
					   int(*compare)( const struct list_head* , const struct list_head* ) )
{
    if( NULL == begin || NULL == end || NULL == compare )
    {
        assert( false );
        return;
    }

    struct list_head *key, *iter, *BeginGuard, *EndGuard;
    
    for( BeginGuard=begin->prev, EndGuard=end->next, key=begin->next, iter=key->prev; 
         key != EndGuard; 
         iter=key, key=key->next )
    {
        while( iter != BeginGuard && compare( iter, key ) > 0 )
        {
            iter = iter->prev;
        }
        if( iter->next != key )
        { 
            // key指向的节点调整到iter的后面
            list_move( key, iter);
       }
    }
}

/**
 * 双向链表插入排序, in ascending order 
 * @head: 待排序链表表头，表示整个链表
 * @compare: compare回调函数，在该函数内计算链表的宿主结构并对宿主的key进行比较，0: equal, <0: less, >0: greater
 */
void list_insert_sort( struct list_head* head, 
					   int(*compare)( const struct list_head* , const struct list_head* ) )
{
    if( NULL == head || NULL == compare )
    {
        assert( false );
        return;
    }

    if( !list_empty(head) )
    {
        __list_insert_sort( head->next, head->prev, compare );
    }
}

