include ../../global.mk

.PHONY: all clean rebuild build_xml

all:
	$(TDR) -H --add_custom_prefix="m_" ./cfg/ZoneHttpConnCfgDesc.xml -O ./cfg  
	$(TDR) -C ./cfg/ZoneHttpConnCfgDesc.xml -o ./cfg/ZoneHttpConnCfgDesc.c 
	$(CC) $(CFLAGS) -o ./cfg/ZoneHttpConnCfgDesc.o -c ./cfg/ZoneHttpConnCfgDesc.c
	$(MAKE) -f makefile.real

clean:
	rm -f ./cfg/*.h ./cfg/*.c ./cfg/*.o
	$(MAKE) -f makefile.real clean

rebuild: clean all

build_xml:
	$(TDR) -H --add_custom_prefix="m_" ./cfg/ZoneHttpConnCfgDesc.xml -O ./cfg  
	$(TDR) -C ./cfg/ZoneHttpConnCfgDesc.xml -o ./cfg/ZoneHttpConnCfgDesc.c 
