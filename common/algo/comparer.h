#pragma once

/* 编译器目前不支持虚函数模板, 使用模版特化技术
   模板特化为引用，指针类型, 请百度
   参考例子: test/TestIndexedPQ.cpp test/TestIndexedPQ_2.cpp
*/

template< typename T >
class Comparer
{
public:
	int Compare( T& x, T& y )
	{
		if( x > y ) return 1;
		if( x < y ) return -1;
		return 0;
	}
};

/*
 	注意默认的行为要求 T和Key是同一类型, 否者运行时发生不可预知错误!!!
*/
template< typename T, typename Key >
class GetItemKey
{
public:
	Key GetKey( const T& elem )
	{
		return (Key)elem;
	}
};

