include ../../global.mk

.PHONY: all clean rebuild build_xml

all:
	$(TDR) -H --add_custom_prefix="m_" ./cfg/CombineSvrCfgDesc.xml -O ./cfg  
	$(TDR) -C ./cfg/CombineSvrCfgDesc.xml -o ./cfg/CombineSvrCfgDesc.c 
	$(CC) $(CFLAGS) -o ./cfg/CombineSvrCfgDesc.o -c ./cfg/CombineSvrCfgDesc.c
	$(MAKE) -f makefile.real

clean:
	rm -f ./cfg/*.h ./cfg/*.c ./cfg/*.o
	$(MAKE) -f makefile.real clean

rebuild: clean all

build_xml:
	$(TDR) -H --add_custom_prefix="m_" ./cfg/CombineSvrCfgDesc.xml -O ./cfg  
	$(TDR) -C ./cfg/CombineSvrCfgDesc.xml -o ./cfg/CombineSvrCfgDesc.c 

