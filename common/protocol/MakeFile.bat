@echo "make proto xml ..... "
@set CUR_DIR=%cd%
@set TDR_EXE=%CUR_DIR%\..\..\..\tools\External\tdr\tdr.exe

%TDR_EXE% -P --add_custom_prefix="m_" common_proto.xml cs_proto.xml
%TDR_EXE% -P --add_custom_prefix="m_" common_proto.xml ss_proto.xml
%TDR_EXE% -P --add_custom_prefix="m_" dwlog_def.xml dwlog_clt.xml
%TDR_EXE% -P --add_custom_prefix="m_" dwlog_def.xml dwlog_svr.xml

pause