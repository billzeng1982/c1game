include ../../global.mk

.PHONY: all clean rebuild

all:
	$(TDR) -H --add_custom_prefix="m_" ./cfg/ClusterAccountSvrCfgDesc.xml -O ./cfg  
	$(TDR) -C ./cfg/ClusterAccountSvrCfgDesc.xml -o ./cfg/ClusterAccountSvrCfgDesc.c 
	$(CC) $(CFLAGS) -o ./cfg/ClusterAccountSvrCfgDesc.o -c ./cfg/ClusterAccountSvrCfgDesc.c
	$(MAKE) -f makefile.real

clean:
	rm -f ./cfg/*.h ./cfg/*.c ./cfg/*.o
	$(MAKE) -f makefile.real clean

rebuild: clean all



