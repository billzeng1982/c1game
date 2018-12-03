#include <stdio.h>

#include "list.h"

#define EX_NODE_NUM 10

class CExBase
{
public:
    CExBase() : m_iData(0) {}
    virtual ~CExBase() { }
        
    virtual int GetData() { return m_iData; }
    virtual void SetData( int iData ) { m_iData = iData; }
private:
    int m_iData;
};


class CExDerive : public CExBase
{
public:
    // embeded data structure MUST-BE declared as public
    struct list_head link;
    
public:
    CExDerive() : m_iData(0) {}
    virtual ~CExDerive() {}

    virtual int GetData() { return m_iData; }
    virtual void SetData( int iData ) { m_iData = iData; }

private:
    int m_iData;
};


int main ()
{
    struct list_head head; // ����ͷ��������������
    INIT_LIST_HEAD( &head );

    CExDerive* poExDeriveIter = NULL;
    CExDerive* paoExDerive = new CExDerive[ EX_NODE_NUM ];
    poExDeriveIter = &paoExDerive[0];
    for( int i = 0; i < EX_NODE_NUM; i++, poExDeriveIter++)
    {
        poExDeriveIter->SetData( i+1 );

        // ��������, add tail
        list_add_tail( &(poExDeriveIter->link), &head );
    }

#if 0
    poExDeriveIter = list_entry( head.next, typeof(*poExDeriveIter), link ); 
    printf("%d \n", poExDeriveIter->GetData() );
#endif

    // ��ӡ
    list_for_each_entry(poExDeriveIter, &head, link)
    {
        printf("%d ", poExDeriveIter->GetData());
    }
    printf("\n");
}


