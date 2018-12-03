include ../../global.mk

.PHONY: all clean rebuild

all:
	$(TDR) -H --add_custom_prefix="m_" ./cfg/NetConnCfgDesc.xml -O ./cfg  
	$(TDR) -C ./cfg/NetConnCfgDesc.xml -o ./cfg/NetConnCfgDesc.c 
	$(CC) $(CFLAGS) -o ./cfg/NetConnCfgDesc.o -c ./cfg/NetConnCfgDesc.c
	$(MAKE) -f makefile.real

clean:
	rm -f ./cfg/*.h ./cfg/*.c ./cfg/*.o
	$(MAKE) -f makefile.real clean

rebuild: clean all

