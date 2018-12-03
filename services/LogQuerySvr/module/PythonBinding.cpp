
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
    //����ʼ��Python���������ͷ���Դ
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
    //��ʼ��python
    Py_Initialize();
    //����ʼ���Ƿ�ɹ�
    if (!Py_IsInitialized())
    {
        LOGERR("Py_Initialize failed");
        return false;
    }

    //��ӵ�ǰ·��
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append('./scripts')");

	return true;
}

//����ű�
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

//���ýű��Ĳ�ѯ��־����
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

    //������������ͷŶ���pRet���������ߣ��ɵ����߸����ͷ�
	if (pArgs)
	{
		Py_DECREF(pModule);
	}

    return pRet;
}
