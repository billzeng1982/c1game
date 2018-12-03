include ../../global.mk

.PHONY: all clean rebuild

all:
	$(TDR) -H --add_custom_prefix="m_" ./cfg/GuildExpeditionSvrCfgDesc.xml -O ./cfg  
	$(TDR) -C ./cfg/GuildExpeditionSvrCfgDesc.xml -o ./cfg/GuildExpeditionSvrCfgDesc.c 
	$(CC) $(CFLAGS) -o ./cfg/GuildExpeditionSvrCfgDesc.o -c ./cfg/GuildExpeditionSvrCfgDesc.c
	$(MAKE) -f makefile.real

clean:
	rm -f ./cfg/*.h ./cfg/*.c ./cfg/*.o
	$(MAKE) -f makefile.real clean

rebuild: clean all