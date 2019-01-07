include ../../global.mk

PROTO_BIN_DIR = ../../deploy/protocol

.PHONY : all clean

all:
	@$(TDR) -B common_proto.xml -o $(PROTO_BIN_DIR)/common_proto.bin
	@$(TDR) -B common_proto.xml cs_proto.xml -o $(PROTO_BIN_DIR)/cs_proto.bin
	@$(TDR) -B common_proto.xml idip_proto.xml -o $(PROTO_BIN_DIR)/idip_proto.bin
	@$(TDR) -B common_proto.xml ss_proto.xml -o $(PROTO_BIN_DIR)/ss_proto.bin
	@$(TDR) -B dwlog_def.xml dwlog_clt.xml -o $(PROTO_BIN_DIR)/dwlog_clt.bin
	@$(TDR) -B dwlog_def.xml dwlog_svr.xml -o $(PROTO_BIN_DIR)/dwlog_svr.bin
	@$(TDR) -B dwlog_def.xml StatsIndicator.xml -o $(PROTO_BIN_DIR)/StatsIndicator.bin
	
	@$(TDR) -P --add_custom_prefix="m_" common_proto.xml cs_proto.xml
	@$(TDR) -P --add_custom_prefix="m_" common_proto.xml idip_proto.xml
	@$(TDR) -P --add_custom_prefix="m_" common_proto.xml ss_proto.xml
	@$(TDR) -P --add_custom_prefix="m_" dwlog_def.xml dwlog_clt.xml
	@$(TDR) -P --add_custom_prefix="m_" dwlog_def.xml dwlog_svr.xml
	
	@$(MAKE) -f makefile.real

clean:
	rm -f *.h *.bin *.tdr
	rm -f $(PROTO_BIN_DIR)/*
	@$(MAKE) -f makefile.real clean
	
test:
	@$(TDR) -B common_proto.xml cs_proto.xml -o $(PROTO_BIN_DIR)/cs_proto.bin

