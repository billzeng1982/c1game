include ../../global.mk

.PHONY: all clean rebuild build_xml

all:
	$(TDR) -H --add_custom_prefix="m_" ./cfg/MessageSvrCfgDesc.xml -O ./cfg  
	$(TDR) -C ./cfg/MessageSvrCfgDesc.xml -o ./cfg/MessageSvrCfgDesc.c 
	$(CC) $(CFLAGS) -o ./cfg/MessageSvrCfgDesc.o -c ./cfg/MessageSvrCfgDesc.c
	$(MAKE) -f makefile.real

clean:
	rm -f ./cfg/*.h ./cfg/*.c ./cfg/*.o
	$(MAKE) -f makefile.real clean

rebuild: clean all

build_xml:
	$(TDR) -H --add_custom_prefix="m_" ./cfg/MessageSvrCfgDesc.xml -O ./cfg  
	$(TDR) -C ./cfg/MessageSvrCfgDesc.xml -o ./cfg/MessageSvrCfgDesc.c 

