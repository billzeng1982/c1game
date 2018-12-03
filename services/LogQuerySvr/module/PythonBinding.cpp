
#include "PythonBinding.h"
#include "LogMacros.h"

PythonBinding::PythonBinding()
{
    pModule = NULL;
    pDict = NULL;
    pFunc = NULL;
    pRet = NULL;
}

PythonBinding::~PythonBinding()
{
    //反初始化Python解释器，释放资源
    Py_Finalize();

	if (pDict)
	{
		Py_DECREF(pDict);
	}

	if (pFunc)
	{
		Py_DECREF(pModule);
	}
}

bool PythonBinding::Init()
{
    //初始化python
    Py_Initialize();
    //检查初始化是否成功
    if (!Py_IsInitialized())
    {
        LOGERR("Py_Initialize failed");
        return false;
    }

    //添加当前路径
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append('./scripts')");

	return true;
}

//载入脚本
bool PythonBinding::LoadScript(const char* pszScriptFile)
{
    pModule = PyImport_ImportModule(pszScriptFile);
    if ( !pModule)
    {
        PyObject *ptype, *pvalue, *ptraceback;
        PyErr_Fetch(&ptype, &pvalue, &ptraceback);
        char *pStrErrorMessage = PyString_AsString(pvalue);
        LOGERR("The output is: %s", pStrErrorMessage);
        LOGERR("load script %s failed", pszScriptFile);
        return false;
    }

    return true;
}

//调用脚本的查询日志函数
PyObject* PythonBinding::func_QueryLog(const char* pszScript, PyObject* pArgs)
{
	//PyGILState_STATE gstate;  
	//gstate = PyGILState_Ensure();

    if (!LoadScript(pszScript))
    {
        return NULL;
    }

    pDict = PyModule_GetDict(pModule);
    pFunc = PyDict_GetItemString(pDict, "QueryLog");

    pRet = PyEval_CallObject(pFunc, pArgs);

    //函数调用完后，释放对象，pRet传给调用者，由调用者负责释放
	if (pArgs)
	{
		Py_DECREF(pModule);
	}

    return pRet;
}
