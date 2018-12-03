include ../../global.mk

.PHONY: all clean rebuild

all:
	$(TDR) -H --add_custom_prefix="m_" ./cfg/ZoneSvrCfgDesc.xml -O ./cfg  
	$(TDR) -C ./cfg/ZoneSvrCfgDesc.xml -o ./cfg/ZoneSvrCfgDesc.c 
	$(CC) $(CFLAGS) -o ./cfg/ZoneSvrCfgDesc.o -c ./cfg/ZoneSvrCfgDesc.c
	$(MAKE) -f makefile.real

clean:
	rm -f ./cfg/*.h ./cfg/*.c ./cfg/*.o ./cfg/*.tdr
	$(MAKE) -f makefile.real clean

rebuild: clean all

