#include <stdio.h>
#include <string.h>
#include "IndexedPriorityQ.h"

// 模板特化为引用，指针类型, 请百度

class Student
{
public:
    int age;
    char name[16];
    int objID;
};

template<> 
class Comparer<Student*>
{
public:
    int Compare( const Student* x, const Student* y )
    {
        if( x->age < y->age )
        {
            return -1;
        }
        if( x->age == y->age )
        {
            return 0;
        }
        return 1;
    }
};

template<> 
class GetItemKey<Student*, int>
{
public: 
    int& GetKey( Student* elem ) 
    { 
        return elem->objID;
    }
};


int main()
{
    IndexedPriorityQ<Student*, int> oPrioQ(10, new Comparer<Student*>(), new GetItemKey<Student*, int>() );

    for( int i = 0; i < 12; i++ )
    {
        Student* student = new Student();

        student->age = 20 - i;
        student->objID = i + 1;
        snprintf(student->name, sizeof(student->name), "Student_%d", student->objID );

        oPrioQ.Push( student );
    }

    for( int i = 0; i < oPrioQ.Length(); i++ )
    {   
        int iObjID = i + 1;
        int iPos = oPrioQ.Find(i+1);
        Student* student = oPrioQ[iPos];
        printf("unikey: %d, pos: %d, age: %d, name: %s \n", iObjID,  iPos, student->age, student->name );
    }
    
    oPrioQ.Erase( 5 );
    oPrioQ.Erase( 10 );
    
    printf("queue contentes:\n");
    while( !oPrioQ.Empty() )
    {
        printf("%s, pos: %d\n ", oPrioQ.Top()->name, oPrioQ.Find( oPrioQ.Top()->objID ));
        
        oPrioQ.Pop();
    }
    printf("\n");

    return 0;
}


