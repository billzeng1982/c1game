include ../../global.mk

.PHONY: all clean rebuild

all:
	$(TDR) -H --add_custom_prefix="m_" ./cfg/LogQuerySvrCfgDesc.xml -O ./cfg  
	$(TDR) -C ./cfg/LogQuerySvrCfgDesc.xml -o ./cfg/LogQuerySvrCfgDesc.c 
	$(CC) $(CFLAGS) -o ./cfg/LogQuerySvrCfgDesc.o -c ./cfg/LogQuerySvrCfgDesc.c
	$(MAKE) -f makefile.real

clean:
	rm -f ./cfg/*.h ./cfg/*.c ./cfg/*.o
	$(MAKE) -f makefile.real clean

rebuild: clean all

