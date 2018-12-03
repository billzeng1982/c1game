#include <assert.h>

#include "list_sort.h"


/**
 * @begin: ����������Ŀ�ʼ��
 * @end: ����������Ľ�����
 * @compare: compare�ص��������ڸú����ڼ�������������ṹ����������key���бȽϣ�0: equal, <0: less, >0: greater
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
            // keyָ��Ľڵ������iter�ĺ���
            list_move( key, iter);
       }
    }
}

/**
 * ˫�������������, in ascending order 
 * @head: �����������ͷ����ʾ��������
 * @compare: compare�ص��������ڸú����ڼ�������������ṹ����������key���бȽϣ�0: equal, <0: less, >0: greater
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

