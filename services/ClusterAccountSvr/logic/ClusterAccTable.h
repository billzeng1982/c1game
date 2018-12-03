#pragma once

/*
	�˺ű����api, ע������api������Reentrant��, ��Ϊ�ڶ��߳���ִ��
*/

#include "define.h"
#include "mysql/DataTable.h"
#include "ss_proto.h"
#include "cfg/ClusterAccountSvrCfgDesc.h"

using namespace PKGMETA;

class ClusterAccTable : public CDataTable
{
public:
    	ClusterAccTable() {}
    	virtual ~ClusterAccTable(){}

    	void SetConfig( CLUSTERACCOUNTSVRCFG* pstConfig ){ m_pstConfig = pstConfig; }

	int CreateNewAccount( SS_PKG_CLUSTER_ACC_NEW_ROLE_REG& rNewRoleReg );
     int ChangeRoleName(uint64_t ullUin, char* szSdkUserName, char* szRoleName );
	int GetAccInfo( char* szSdkUserName, int iServerID, SS_PKG_GET_ONE_CLUSTER_ACC_INFO_RSP& rstGetAccInfoRsp );

private:
    CLUSTERACCOUNTSVRCFG* m_pstConfig;
};

