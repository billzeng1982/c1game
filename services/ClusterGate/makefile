include ../../global.mk

.PHONY: all clean rebuild

all:
	$(TDR) -H --add_custom_prefix="m_" ./cfg/ClusterGateCfgDesc.xml -O ./cfg  
	$(TDR) -C ./cfg/ClusterGateCfgDesc.xml -o ./cfg/ClusterGateCfgDesc.c 
	$(CC) $(CFLAGS) -o ./cfg/ClusterGateCfgDesc.o -c ./cfg/ClusterGateCfgDesc.c
	$(MAKE) -f makefile.real

clean:
	rm -f ./cfg/*.h ./cfg/*.c ./cfg/*.o ./cfg/*.tdr
	$(MAKE) -f makefile.real clean

rebuild: clean all

