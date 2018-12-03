include ../../global.mk

.PHONY: all clean rebuild

all:
	$(TDR) -H --add_custom_prefix="m_" ./cfg/MineSvrCfgDesc.xml -O ./cfg  
	$(TDR) -C ./cfg/MineSvrCfgDesc.xml -o ./cfg/MineSvrCfgDesc.c 
	$(CC) $(CFLAGS) -o ./cfg/MineSvrCfgDesc.o -c ./cfg/MineSvrCfgDesc.c
	$(MAKE) -f makefile.real

clean:
	rm -f ./cfg/*.h ./cfg/*.c ./cfg/*.o ./cfg/*.tdr
	$(MAKE) -f makefile.real clean

rebuild: clean all

