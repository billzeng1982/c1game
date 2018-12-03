#ifndef _GGAME_FIFO_H_
#define _GGAME_FIFO_H_

/*
	���ζ���

	1. ����������õ����ݱ�����POD��,��Ϊ����copyʱʹ�õ���memcpy
	2. һ������һ��д�ߵ�������������
	3. �㷨copy��linux-2.6.37 kfifo

	Լ��:
	���д�С(size)������2��n�η�!

	����unsigned int��ע��:
	1. �޷�������������Զ�����
	2. �޷������� С-�� = 0xFFFFFFFF - (��-1) + С = 0xFFFFFFFF - ( �� - С ) + 1
*/

#include <assert.h>
#include <string.h>
#include "define.h"

template< typename T >
class CFifo
{
public:
	CFifo()
	{
		in 		= 0;
		out 	= 0;
		mask 	= 0;
		esize 	= sizeof( T );
		data 	= NULL;
	}

	~CFifo()
	{
		if( data )
		{
			delete [] data;
		}
	}

	CFifo( uint32_t size )
	{
		if( (size < 2) || (size & (size - 1)) )
		{
			// ����2��n�η�
			assert( false );
			data = NULL;
			return;
		}

		data = new T[size];
		if( !data )
		{
			assert( false );
			return;
		}

		in = 0;
		out = 0;
		mask = size - 1;
		esize 	= sizeof( T );
	}

	// data��shm�ṩ
	bool AttachShm( uint32_t size, void* shm )
	{
		if( (size < 2) || (size & (size - 1)) )
		{
			// ����2��n�η�
			assert( false );
			data = NULL;
			return false;
		}

		in = 0;
		out = 0;
		mask = size - 1;
		esize 	= sizeof( T );
		data = (T*)shm;
	}

	// �����ӿ�
	uint32_t FifoIn( const T* buf, uint32_t len );

	uint32_t FifoOut( T* buf, uint32_t len );

	uint32_t FifoOutPeek( T* buf, uint32_t len );

	uint32_t FifoUnused()
	{
		return (mask + 1) - (in - out);
	}


private:
	void _FifoCopyOut( T* dst, uint32_t len, uint32_t off );
	void _FifoCopyIn( const T* src, uint32_t len, uint32_t off );

private:
	uint32_t	in;
	uint32_t	out;
	uint32_t	mask; 	// = size - 1; size = 2^n
	uint32_t	esize; 	// element size = sizeof( T )
	T*			data;
};


template< typename T >
uint32_t CFifo<T>::FifoIn( const T* buf, uint32_t len )
{
	uint32_t l;

	l = FifoUnused( );
	if (len > l)
		len = l;

	_FifoCopyIn(buf, len, this->in);
	this->in += len;
	return len;
}


template< typename T >
uint32_t CFifo<T>::FifoOut( T* buf, uint32_t len )
{
	len = FifoOutPeek(buf, len);
	this->out += len;
	return len;
}


template< typename T >
uint32_t CFifo<T>::FifoOutPeek( T* buf, uint32_t len )
{
	unsigned int l;

	l = this->in - this->out;
	if (len > l)
		len = l;

	_FifoCopyOut(buf, len, this->out);
	return len;
}


template< typename T >
void CFifo<T>::_FifoCopyOut( T* dst, uint32_t len, uint32_t off )
{
	unsigned int size = this->mask + 1;
	unsigned int l;

	off &= this->mask;
	l = MIN(len, size - off);

	memcpy(dst, this->data + off, l * this->esize);
	memcpy(dst + l, this->data, (len - l) * this->esize);
	/*
	 * make sure that the data is copied before
	 * incrementing the fifo->out index counter
	 */
}


template< typename T >
void CFifo<T>::_FifoCopyIn( const T* src, uint32_t len, uint32_t off )
{
	uint32_t size = this->mask + 1;
	uint32_t l;

	off &= this->mask;
	l = MIN(len, size - off);

	memcpy(this->data + off, src, l * this->esize);
	memcpy(this->data, src + l, (len - l) * this->esize);
	/*
	 * make sure that the data in the fifo is up to date before
	 * incrementing the fifo->in index counter
	 */
}


#endif

