include ../../global.mk

.PHONY: all clean rebuild build_xml

all:
	$(TDR) -H --add_custom_prefix="m_" ./cfg/GuildSvrCfgDesc.xml -O ./cfg  
	$(TDR) -C ./cfg/GuildSvrCfgDesc.xml -o ./cfg/GuildSvrCfgDesc.c 
	$(CC) $(CFLAGS) -o ./cfg/GuildSvrCfgDesc.o -c ./cfg/GuildSvrCfgDesc.c
	$(MAKE) -f makefile.real

clean:
	rm -f ./cfg/*.h ./cfg/*.c ./cfg/*.o
	$(MAKE) -f makefile.real clean

rebuild: clean all

build_xml:
	$(TDR) -H --add_custom_prefix="m_" ./cfg/GuildSvrCfgDesc.xml -O ./cfg  
	$(TDR) -C ./cfg/GuildSvrCfgDesc.xml -o ./cfg/GuildSvrCfgDesc.c 

