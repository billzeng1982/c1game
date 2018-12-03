#ifndef _TSINGLETON_H_
#define _TSINGLETON_H_

#include <cstddef>

// 老版本,"假"单例, 可做全局变量来使用
template<typename T>
class   CSingleton
{
private:
    static T m_oObj;

public:
    static T& Instance()
    {
        return  m_oObj;
    }
};

template<typename T>
T CSingleton<T>::m_oObj;

#define SINGLE_INSTANCE( TClass ) CSingleton<TClass>::Instance()


// 新版本, 继承使用
template<typename T>
class TSingleton
{
protected:
	TSingleton() {}
	virtual ~TSingleton() {}

private:
	TSingleton(const TSingleton&);
    TSingleton& operator=(const TSingleton&);

public:
	static T& Instance()
	{
		if( !m_inst ) 
		{
			m_inst= new T;
		}
		return *m_inst;
	}

private:
	static T* m_inst;
};

template<typename T>
T* TSingleton<T>::m_inst = NULL;

#endif

