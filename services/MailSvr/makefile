include ../../global.mk

.PHONY: all clean rebuild

all:
	$(TDR) -H --add_custom_prefix="m_" ./cfg/MailSvrCfgDesc.xml -O ./cfg  
	$(TDR) -C ./cfg/MailSvrCfgDesc.xml -o ./cfg/MailSvrCfgDesc.c 
	$(CC) $(CFLAGS) -o ./cfg/MailSvrCfgDesc.o -c ./cfg/MailSvrCfgDesc.c
	$(MAKE) -f makefile.real

clean:
	rm -f ./cfg/*.h ./cfg/*.c ./cfg/*.o ./cfg/*.tdr
	$(MAKE) -f makefile.real clean

rebuild: clean all

