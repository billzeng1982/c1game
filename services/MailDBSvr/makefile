include ../../global.mk

.PHONY: all clean rebuild

all:
	$(TDR) -H --add_custom_prefix="m_" ./cfg/MailDBSvrCfgDesc.xml -O ./cfg  
	$(TDR) -C ./cfg/MailDBSvrCfgDesc.xml -o ./cfg/MailDBSvrCfgDesc.c 
	$(CC) $(CFLAGS) -o ./cfg/MailDBSvrCfgDesc.o -c ./cfg/MailDBSvrCfgDesc.c
	$(MAKE) -f makefile.real

clean:
	rm -f ./cfg/*.h ./cfg/*.c ./cfg/*.o
	$(MAKE) -f makefile.real clean

rebuild: clean all

