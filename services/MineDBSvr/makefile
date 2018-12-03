include ../../global.mk

.PHONY: all clean rebuild

all:
	$(TDR) -H --add_custom_prefix="m_" ./cfg/MineDBSvrCfgDesc.xml -O ./cfg  
	$(TDR) -C ./cfg/MineDBSvrCfgDesc.xml -o ./cfg/MineDBSvrCfgDesc.c 
	$(CC) $(CFLAGS) -o ./cfg/MineDBSvrCfgDesc.o -c ./cfg/MineDBSvrCfgDesc.c
	$(MAKE) -f makefile.real

clean:
	rm -f ./cfg/*.h ./cfg/*.c ./cfg/*.o
	$(MAKE) -f makefile.real clean

rebuild: clean all