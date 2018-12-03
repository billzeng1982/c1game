include ../../global.mk

.PHONY: all clean rebuild

all:
	$(TDR) -H --add_custom_prefix="m_" ./cfg/ClusterDBSvrCfgDesc.xml -O ./cfg  
	$(TDR) -C ./cfg/ClusterDBSvrCfgDesc.xml -o ./cfg/ClusterDBSvrCfgDesc.c 
	$(CC) $(CFLAGS) -o ./cfg/ClusterDBSvrCfgDesc.o -c ./cfg/ClusterDBSvrCfgDesc.c
	$(MAKE) -f makefile.real

clean:
	rm -f ./cfg/*.h ./cfg/*.c ./cfg/*.o
	$(MAKE) -f makefile.real clean

rebuild: clean all