#pragma once

#include <Python.h>
#include "singleton.h"

class PythonBinding :  public TSingleton<PythonBinding>
{
public:
    PythonBinding();
    ~PythonBinding();

    bool Init();

    PyObject * func_QueryLog(const char* pszScript, PyObject* pArgs);

protected:
    bool LoadScript(const char* pszScriptFile);

private:
    PyObject * pModule;
    PyObject * pDict;
    PyObject * pFunc;
    PyObject * pRet;
};
