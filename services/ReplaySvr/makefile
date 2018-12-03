include ../../global.mk

.PHONY: all clean rebuild

all:
	$(TDR) -H --add_custom_prefix="m_" ./cfg/ReplaySvrCfgDesc.xml -O ./cfg  
	$(TDR) -C ./cfg/ReplaySvrCfgDesc.xml -o ./cfg/ReplaySvrCfgDesc.c 
	$(CC) $(CFLAGS) -o ./cfg/ReplaySvrCfgDesc.o -c ./cfg/ReplaySvrCfgDesc.c
	$(MAKE) -f makefile.real

clean:
	rm -f ./cfg/*.h ./cfg/*.c ./cfg/*.o ./cfg/*.tdr
	$(MAKE) -f makefile.real clean

rebuild: clean all

