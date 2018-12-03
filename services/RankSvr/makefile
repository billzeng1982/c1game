include ../../global.mk

.PHONY: all clean rebuild

all:
	$(TDR) -H --add_custom_prefix="m_" ./cfg/RankSvrCfgDesc.xml -O ./cfg  
	$(TDR) -C ./cfg/RankSvrCfgDesc.xml -o ./cfg/RankSvrCfgDesc.c 
	$(CC) $(CFLAGS) -o ./cfg/RankSvrCfgDesc.o -c ./cfg/RankSvrCfgDesc.c
	$(MAKE) -f makefile.real

clean:
	rm -f ./cfg/*.h ./cfg/*.c ./cfg/*.o
	$(MAKE) -f makefile.real clean

rebuild: clean all

