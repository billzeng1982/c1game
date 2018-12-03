#pragma once

/* ������Ŀǰ��֧���麯��ģ��, ʹ��ģ���ػ�����
   ģ���ػ�Ϊ���ã�ָ������, ��ٶ�
   �ο�����: test/TestIndexedPQ.cpp test/TestIndexedPQ_2.cpp
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
 	ע��Ĭ�ϵ���ΪҪ�� T��Key��ͬһ����, ��������ʱ��������Ԥ֪����!!!
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

