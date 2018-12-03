#ifndef _FUNCTORS_H_
#define _FUNCTORS_H_

/*
	¸÷ÖÖ·Âº¯Êýstruct
*/

#include <string.h>

template< typename T >
struct SDftLess
{
	bool operator() ( const T& lhs, const T& rhs ) const
	{
		return ( lhs < rhs );
	}
};


template< typename T >
struct SDftEqual
{
	bool operator() ( const T& lhs, const T& rhs ) const
	{
		return ( lhs == rhs );
	}
};


template< typename T >
struct SDftGreater
{
	bool operator() ( const T& lhs, const T& rhs ) const
	{
		return ( lhs > rhs );
	}
};


template< typename T >
struct SDftCompare
{
	int operator() ( const T& lhs, const T& rhs ) const
	{
		if( lhs == rhs )
		{
			return 0;
		}
		else if( lhs > rhs )
		{
			return 1;
		}
		else
		{
			return -1;
		}
	}
};


template< typename T >
struct SDftGetKey
{
	const T& operator() ( const T& rhs )
	{
		return rhs;
	}
};


struct eqstr
{
	bool operator()(const char* s1, const char* s2) const
	{
		return strcmp(s1, s2) == 0;
	}
};


struct ltstr
{
	bool operator()(const char* s1, const char* s2) const
	{
		return strcmp(s1, s2) < 0;
	}
};

#endif

