include ../../global.mk

.PHONY: all clean rebuild

all:
	$(TDR) -H --add_custom_prefix="m_" ./cfg/AccountSvrCfgDesc.xml -O ./cfg  
	$(TDR) -C ./cfg/AccountSvrCfgDesc.xml -o ./cfg/AccountSvrCfgDesc.c 
	$(CC) $(CFLAGS) -o ./cfg/AccountSvrCfgDesc.o -c ./cfg/AccountSvrCfgDesc.c
	$(MAKE) -f makefile.real

clean:
	rm -f ./cfg/*.h ./cfg/*.c ./cfg/*.o
	$(MAKE) -f makefile.real clean

rebuild: clean all

