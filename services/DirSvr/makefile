include ../../global.mk

.PHONY: all clean rebuild

all:
	$(TDR) -H --add_custom_prefix="m_" ./cfg/DirSvrCfgDesc.xml -O ./cfg  
	$(TDR) -C ./cfg/DirSvrCfgDesc.xml -o ./cfg/DirSvrCfgDesc.c 
	$(CC) $(CFLAGS) -o ./cfg/DirSvrCfgDesc.o -c ./cfg/DirSvrCfgDesc.c
	$(MAKE) -f makefile.real

clean:
	rm -f ./cfg/*.h ./cfg/*.c ./cfg/*.o ./cfg/*.tdr
	$(MAKE) -f makefile.real clean

rebuild: clean all

