include ../../global.mk

.PHONY: all clean rebuild xml

all:
	$(TDR) -H --add_custom_prefix="m_" ./cfg/SdkDMSvrCfgDesc.xml -O ./cfg  
	$(TDR) -C ./cfg/SdkDMSvrCfgDesc.xml -o ./cfg/SdkDMSvrCfgDesc.c 
	$(CC) $(CFLAGS) -o ./cfg/SdkDMSvrCfgDesc.o -c ./cfg/SdkDMSvrCfgDesc.c
	$(MAKE) -f makefile.real

xml:
	$(TDR) -H --add_custom_prefix="m_" ./cfg/SdkDMSvrCfgDesc.xml -O ./cfg  
	$(TDR) -C ./cfg/SdkDMSvrCfgDesc.xml -o ./cfg/SdkDMSvrCfgDesc.c 
	
clean:
	rm -f ./cfg/*.h ./cfg/*.c ./cfg/*.o
	$(MAKE) -f makefile.real clean

rebuild: clean all

