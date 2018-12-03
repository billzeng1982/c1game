#include "functional.h"

using namespace RayE;

class Add
{
public:
    int add(int a, int b)
    {
        return a+b;
    }
};

static int add(int a, int b)
{
    return a+b;
}

class TestFunction1
{
public:
    void test( int a )
    {
        printf("a=%d\n", a);
    }

    void bind()
    {
        f = RayE::Bind( &TestFunction1::test, this );
    }

    Function1<void, int> f;
};

static void test( Function2<int, int, int> g )
{
    printf("g(5,6)=%d\n", g(5,6));
}

int main()
{
    Add a;
    Function2<int, int, int> f;

    f = Bind(&Add::add, &a);

    int iRet = f(2, 4);
    printf("iRet = %d\n", iRet);

    Function2<int, int, int> g=f;
    printf("g(3,4)=%d\n", g(3,4));

    test( f );

    printf("f(3,4)=%d\n", f(3,4));

    f = Bind(&add);
    iRet = f(5,10);

    printf("iRet = %d\n", iRet);

    //TestFunction1 test1;
    //test1.bind();
    //test1.f(20);

    return 0;
}

