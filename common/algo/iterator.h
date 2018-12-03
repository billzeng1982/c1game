#ifndef _ITERATOR_H_
#define _ITERATOR_H_

/*
	iterator 基类
	iterator 模式 ...
*/

template< typename T > 
class IIterator
{
public:
	virtual ~IIterator(){}

	virtual void Begin() = 0;
	virtual void Next()  = 0;
	virtual bool IsEnd() = 0;
	virtual T	 CurrItem() = 0;
	
protected:
	IIterator() {}
};

#endif

