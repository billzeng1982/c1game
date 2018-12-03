include ../../global.mk

.PHONY: all clean rebuild

all:
	$(TDR) -H --add_custom_prefix="m_" ./cfg/FightSvrCfgDesc.xml -O ./cfg  
	$(TDR) -C ./cfg/FightSvrCfgDesc.xml -o ./cfg/FightSvrCfgDesc.c 
	$(CC) $(CFLAGS) -o ./cfg/FightSvrCfgDesc.o -c ./cfg/FightSvrCfgDesc.c
	$(MAKE) -f makefile.real

clean:
	rm -f ./cfg/*.h ./cfg/*.c ./cfg/*.o ./cfg/*.tdr
	$(MAKE) -f makefile.real clean

rebuild: clean all

