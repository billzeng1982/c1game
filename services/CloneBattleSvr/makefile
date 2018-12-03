include ../../global.mk

.PHONY: all clean rebuild

all:
	$(TDR) -H --add_custom_prefix="m_" ./cfg/CloneBattleSvrCfgDesc.xml -O ./cfg  
	$(TDR) -C ./cfg/CloneBattleSvrCfgDesc.xml -o ./cfg/CloneBattleSvrCfgDesc.c 
	$(CC) $(CFLAGS) -o ./cfg/CloneBattleSvrCfgDesc.o -c ./cfg/CloneBattleSvrCfgDesc.c
	$(MAKE) -f makefile.real

clean:
	rm -f ./cfg/*.h ./cfg/*.c ./cfg/*.o ./cfg/*.tdr
	$(MAKE) -f makefile.real clean

rebuild: clean all

