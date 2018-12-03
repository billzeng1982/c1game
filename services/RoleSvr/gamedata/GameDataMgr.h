#pragma once

#include "TGameDataMgr.h"
#include "ov_res_keywords.h"
#include "ov_res_public.h"
#include "ov_res_server.h"
#include "singleton.h"
#include "macros.h"


/*
	Ŀǰ������ʱȫ����������gamedata,�����ڴ��Ż�ʱ���趯̬���أ�ж��
*/

// using namespace std;
// 
// typedef TGameDataMgr<RESPUBMAIL>		ResPubMailMgr_t;
// typedef TGameDataMgr<RESPRIMAIL>		ResPriMailMgr_t;
// 
// class CGameDataMgr : public TSingleton<CGameDataMgr>
// {
// public:
// 	CGameDataMgr(){}
// 	~CGameDataMgr(){}
// 
// 	bool Init();
// 
// private:
// 	DECL_INIT_RES_MGR( m_oResPubMailMgr,	"gamedata/ResData/server/mail/ResPubMail.bytes" );
// 	DECL_INIT_RES_MGR( m_oResPriMailMgr,	"gamedata/ResData/server/mail/ResPriMail.bytes" );
// 	
// private:
// 	DECL_GETTER_REF( ResPubMailMgr_t,	m_oResPubMailMgr,	ResPubMailMgr );
// 	DECL_GETTER_REF( ResPriMailMgr_t,	m_oResPriMailMgr,	ResPriMailMgr );
// };

