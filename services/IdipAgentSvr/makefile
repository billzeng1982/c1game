include ../../global.mk

.PHONY: all clean rebuild

all:
	$(TDR) -H --add_custom_prefix="m_" ./cfg/IdipAgentSvrCfgDesc.xml -O ./cfg  
	$(TDR) -C ./cfg/IdipAgentSvrCfgDesc.xml -o ./cfg/IdipAgentSvrCfgDesc.c 
	$(CC) $(CFLAGS) -o ./cfg/IdipAgentSvrCfgDesc.o -c ./cfg/IdipAgentSvrCfgDesc.c
	$(MAKE) -f makefile.real

clean:
	rm -f ./cfg/*.h ./cfg/*.c ./cfg/*.o
	$(MAKE) -f makefile.real clean

rebuild: clean all

