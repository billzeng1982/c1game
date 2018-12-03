include ../../global.mk

.PHONY: all clean rebuild

all:
	$(TDR) -H --add_custom_prefix="m_" ./cfg/MiscSvrCfgDesc.xml -O ./cfg  
	$(TDR) -C ./cfg/MiscSvrCfgDesc.xml -o ./cfg/MiscSvrCfgDesc.c 
	$(CC) $(CFLAGS) -o ./cfg/MiscSvrCfgDesc.o -c ./cfg/MiscSvrCfgDesc.c
	$(MAKE) -f makefile.real

clean:
	rm -f ./cfg/*.h ./cfg/*.c ./cfg/*.o
	$(MAKE) -f makefile.real clean

rebuild: clean all