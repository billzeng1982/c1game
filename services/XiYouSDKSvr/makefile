include ../../global.mk

.PHONY: all clean rebuild

all:
	$(TDR) -H --add_custom_prefix="m_" ./cfg/XiYouSDKSvrCfgDesc.xml -O ./cfg  
	$(TDR) -C ./cfg/XiYouSDKSvrCfgDesc.xml -o ./cfg/XiYouSDKSvrCfgDesc.c 
	$(CC) $(CFLAGS) -o ./cfg/XiYouSDKSvrCfgDesc.o -c ./cfg/XiYouSDKSvrCfgDesc.c
	$(MAKE) -f makefile.real

clean:
	rm -f ./cfg/*.h ./cfg/*.c ./cfg/*.o
	$(MAKE) -f makefile.real clean

rebuild: clean all

