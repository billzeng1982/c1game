#ifndef _MACROS_H_
#define _MACROS_H_

#define DECL_GETTER_SETTER(varType, varName, funName)\
protected: varType varName;\
public:  varType Get##funName(void) const { return varName; }\
public:  void Set##funName(varType var){ varName = var; }

#define DECL_GETTER_SETTER_REF(varType, varName, funName)\
protected: varType varName;\
public: varType& Get##funName(void) { return varName; }\
public: void Set##funName(varType& var){ varName = var; }

#define DECL_GETTER_REF(varType, varName, funName) \
protected: varType varName;\
public: varType& Get##funName(void) { return varName; }\

#define DECL_GETTER_SETTER_PTR(varType, varName, funName)\
protected: varType varName;\
public:  varType* Get##funName(void) { return &varName; }\
public:  void Set##funName(varType* var){ varName = *var; }


#endif

