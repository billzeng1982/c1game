#pragma once

#include <stdio.h>
#include <new>
#include <string.h>

/*
	函数绑定模板

	类由宏FUNCTIANAL_DELEGATE生成
	生成的类名为 Function{N}, 其中N表示函数参数个数, 如: Function0, Function1, Function2 等等
	Bind函数不带数字, 直接函数重载

	Function对象定义:
	FunctionN<ReturnType, ParaType1, ... , ParaTypeN> oFunc;

	如:
	Function0<int> oFunc;
	Function2<int, int, int> oFunc;

	Bind使用参看test/TestFunction.cpp
	注意Bind的返回值不能赋值给引用, 因为Bind返回的是临时变量

	内存管理已优化，使用 replacement new, 且相对原先的实现，赋值对象不受影响，同样可以调用 - billzeng 2017/1/2
*/

namespace RayE
{

#define TYPENAME_TEMPLATE_LIST_0 
#define TYPENAME_TEMPLATE_LIST_1 TYPENAME_TEMPLATE_LIST_0, typename _T1
#define TYPENAME_TEMPLATE_LIST_2 TYPENAME_TEMPLATE_LIST_1, typename _T2
#define TYPENAME_TEMPLATE_LIST_3 TYPENAME_TEMPLATE_LIST_2, typename _T3
#define TYPENAME_TEMPLATE_LIST_4 TYPENAME_TEMPLATE_LIST_3, typename _T4

#define ARG_TYPE_LIST_0
#define ARG_TYPE_LIST_1 _T1
#define ARG_TYPE_LIST_2 ARG_TYPE_LIST_1, _T2
#define ARG_TYPE_LIST_3 ARG_TYPE_LIST_2, _T3
#define ARG_TYPE_LIST_4 ARG_TYPE_LIST_3, _T4

#define ARG_TYPE_LIST_0_COMMA 
#define ARG_TYPE_LIST_1_COMMA ARG_TYPE_LIST_0_COMMA, _T1
#define ARG_TYPE_LIST_2_COMMA ARG_TYPE_LIST_1_COMMA, _T2
#define ARG_TYPE_LIST_3_COMMA ARG_TYPE_LIST_2_COMMA, _T3
#define ARG_TYPE_LIST_4_COMMA ARG_TYPE_LIST_3_COMMA, _T4

#define ARG_PARAM_LIST_0
#define ARG_PARAM_LIST_1 arg1
#define ARG_PARAM_LIST_2 ARG_PARAM_LIST_1, arg2
#define ARG_PARAM_LIST_3 ARG_PARAM_LIST_2, arg3
#define ARG_PARAM_LIST_4 ARG_PARAM_LIST_3, arg4

#define ARG_TYPE_PARAM_LIST_0
#define ARG_TYPE_PARAM_LIST_1 _T1 arg1
#define ARG_TYPE_PARAM_LIST_2 ARG_TYPE_PARAM_LIST_1, _T2 arg2
#define ARG_TYPE_PARAM_LIST_3 ARG_TYPE_PARAM_LIST_2, _T3 arg3
#define ARG_TYPE_PARAM_LIST_4 ARG_TYPE_PARAM_LIST_3, _T3 arg4


#define FUNCTIANAL_DELEGATE(ARG_NUM, TYPENAME_TEMPLATE_LIST, ARG_TYPE_LIST, ARG_TYPE_LIST_COMMA, ARG_PARAM_LIST, ARG_TYPE_PARAM_LIST) \
\
template<typename _ReturnType TYPENAME_TEMPLATE_LIST> \
class FunctionBase##ARG_NUM \
{ \
public: \
	FunctionBase##ARG_NUM() { }\
	virtual ~FunctionBase##ARG_NUM() {} \
    	virtual _ReturnType operator()(ARG_TYPE_PARAM_LIST) = 0; \
}; \
\
template<typename _ReturnType, typename _Class TYPENAME_TEMPLATE_LIST> \
class FunctionOfMem##ARG_NUM : public FunctionBase##ARG_NUM<_ReturnType ARG_TYPE_LIST_COMMA> \
{\
public: \
    typedef _ReturnType(_Class::*MemFuncType)(ARG_TYPE_LIST);\
 \
    MemFuncType m_pfunc;\
    _Class* m_obj;\
 \
    FunctionOfMem##ARG_NUM(MemFuncType func, _Class* obj)\
    {\
        m_pfunc = func;\
        m_obj = obj;\
    }\
\
    virtual _ReturnType operator()(ARG_TYPE_PARAM_LIST)\
    {\
        return (m_obj->*m_pfunc)(ARG_PARAM_LIST); \
	} \
};\
\
template<typename _ReturnType TYPENAME_TEMPLATE_LIST> \
class FunctionOfPtr##ARG_NUM : public FunctionBase##ARG_NUM<_ReturnType ARG_TYPE_LIST_COMMA> \
{ \
public: \
	typedef _ReturnType(*PtrFuncType)(ARG_TYPE_LIST); \
\
	PtrFuncType m_pfunc; \
\
    FunctionOfPtr##ARG_NUM(PtrFuncType pf) \
    {\
        m_pfunc = pf; \
    }\
\
    virtual _ReturnType operator()(ARG_TYPE_PARAM_LIST) \
	{\
        return (*m_pfunc)(ARG_PARAM_LIST);\
    }   \
};\
\
template<typename _ReturnType TYPENAME_TEMPLATE_LIST> \
class Function##ARG_NUM \
{ \
public: \
	char m_buf[32]; \
public: \
    FunctionBase##ARG_NUM<_ReturnType ARG_TYPE_LIST_COMMA>* m_pfunc; \
 \
    Function##ARG_NUM() \
    { bzero(m_buf, sizeof(m_buf)); m_pfunc = NULL; printf("default constructor \n");} \
    ~Function##ARG_NUM() \
    {\
        if (m_pfunc != NULL)\
        {\
            /*delete m_pfunc;*/ \
            m_pfunc = NULL;\
        }\
    }\
\
	template<typename _Class>\
    Function##ARG_NUM( _ReturnType(_Class::*pMemFunc)(ARG_TYPE_LIST), _Class* obj )\
    {\
        m_pfunc = ::new(m_buf) FunctionOfMem##ARG_NUM<_ReturnType, _Class ARG_TYPE_LIST_COMMA>(pMemFunc, obj);\
    }\
 \
    Function##ARG_NUM( _ReturnType(*pPtrFunc)(ARG_TYPE_LIST) ) \
    { \
        m_pfunc = ::new(m_buf) FunctionOfPtr##ARG_NUM<_ReturnType ARG_TYPE_LIST_COMMA>(pPtrFunc); \
    } \
 \
    _ReturnType operator()(ARG_TYPE_PARAM_LIST) \
    {\
        return (*m_pfunc)(ARG_PARAM_LIST);\
    }\
\
	/* 赋值构造函数, 注意重载operator=, const必须加，注意声明格式!!*/ \
	Function##ARG_NUM& operator=(const Function##ARG_NUM& f) \
    {\
		Function##ARG_NUM* pf = const_cast<Function##ARG_NUM*>(&f);\
        	/*if( this->m_pfunc )\
		{\
			delete this->m_pfunc; \
			this->m_pfunc = NULL;\
		}\
		this->m_pfunc = pf->m_pfunc;\
		pf->m_pfunc = NULL;*/ \
		printf("= constructor !!\n");\
		m_pfunc = (FunctionBase##ARG_NUM<_ReturnType ARG_TYPE_LIST_COMMA>*)(&m_buf[0]);\
		memcpy( this->m_buf, pf->m_buf, sizeof(m_buf));\
		return *this;\
    }\
\
	/*拷贝构造函数*/ \
	Function##ARG_NUM( const Function##ARG_NUM& f ) \
	{\
		Function##ARG_NUM* pf = const_cast<Function##ARG_NUM*>(&f);\
        	/*if( this->m_pfunc )\
		{\
			delete this->m_pfunc; \
			this->m_pfunc = NULL;\
		}\
		this->m_pfunc = pf->m_pfunc;\
		pf->m_pfunc = NULL;*/ \
		printf("copy constructor !!\n");\
		m_pfunc = (FunctionBase##ARG_NUM<_ReturnType ARG_TYPE_LIST_COMMA>*)(&m_buf[0]);\
		memcpy( this->m_buf, pf->m_buf, sizeof(m_buf)); \
	}\
};\
\
template<typename _ReturnType, typename _Class TYPENAME_TEMPLATE_LIST> \
Function##ARG_NUM<_ReturnType ARG_TYPE_LIST_COMMA> Bind(_ReturnType(_Class::*pMemFunc)(ARG_TYPE_LIST), _Class* obj) \
{ \
    return Function##ARG_NUM<_ReturnType ARG_TYPE_LIST_COMMA>(pMemFunc, obj); \
} \
\
template<typename _ReturnType TYPENAME_TEMPLATE_LIST> \
Function##ARG_NUM<_ReturnType ARG_TYPE_LIST_COMMA> Bind(_ReturnType(*pFunc)(ARG_TYPE_LIST)) \
{\
    return Function##ARG_NUM<_ReturnType ARG_TYPE_LIST_COMMA>(pFunc); \
}\


/*生成类 Function{N}*/
FUNCTIANAL_DELEGATE(0,TYPENAME_TEMPLATE_LIST_0,ARG_TYPE_LIST_0,ARG_TYPE_LIST_0_COMMA,ARG_PARAM_LIST_0,ARG_TYPE_PARAM_LIST_0)
FUNCTIANAL_DELEGATE(1,TYPENAME_TEMPLATE_LIST_1,ARG_TYPE_LIST_1,ARG_TYPE_LIST_1_COMMA,ARG_PARAM_LIST_1,ARG_TYPE_PARAM_LIST_1)
FUNCTIANAL_DELEGATE(2,TYPENAME_TEMPLATE_LIST_2,ARG_TYPE_LIST_2,ARG_TYPE_LIST_2_COMMA,ARG_PARAM_LIST_2,ARG_TYPE_PARAM_LIST_2)
FUNCTIANAL_DELEGATE(3,TYPENAME_TEMPLATE_LIST_3,ARG_TYPE_LIST_3,ARG_TYPE_LIST_3_COMMA,ARG_PARAM_LIST_3,ARG_TYPE_PARAM_LIST_3)
FUNCTIANAL_DELEGATE(4,TYPENAME_TEMPLATE_LIST_4,ARG_TYPE_LIST_4,ARG_TYPE_LIST_4_COMMA,ARG_PARAM_LIST_4,ARG_TYPE_PARAM_LIST_4)
 
#undef TYPENAME_TEMPLATE_LIST_0
#undef TYPENAME_TEMPLATE_LIST_1
#undef TYPENAME_TEMPLATE_LIST_2
#undef TYPENAME_TEMPLATE_LIST_3
#undef TYPENAME_TEMPLATE_LIST_4 
 
#undef ARG_TYPE_LIST_0
#undef ARG_TYPE_LIST_1
#undef ARG_TYPE_LIST_2
#undef ARG_TYPE_LIST_3
#undef ARG_TYPE_LIST_4
 
#undef ARG_PARAM_LIST_0
#undef ARG_PARAM_LIST_1
#undef ARG_PARAM_LIST_2
#undef ARG_PARAM_LIST_3
#undef ARG_PARAM_LIST_4 
 
#undef ARG_TYPE_LIST_0_COMMA 
#undef ARG_TYPE_LIST_1_COMMA 
#undef ARG_TYPE_LIST_2_COMMA 
#undef ARG_TYPE_LIST_3_COMMA 
#undef ARG_TYPE_LIST_4_COMMA  
 
#undef ARG_TYPE_PARAM_LIST_0
#undef ARG_TYPE_PARAM_LIST_1
#undef ARG_TYPE_PARAM_LIST_2
#undef ARG_TYPE_PARAM_LIST_3
#undef ARG_TYPE_PARAM_LIST_4 

};

