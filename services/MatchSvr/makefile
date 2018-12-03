include ../../global.mk

.PHONY: all clean rebuild

all:
	$(TDR) -H --add_custom_prefix="m_" ./cfg/MatchSvrCfgDesc.xml -O ./cfg  
	$(TDR) -C ./cfg/MatchSvrCfgDesc.xml -o ./cfg/MatchSvrCfgDesc.c 
	$(CC) $(CFLAGS) -o ./cfg/MatchSvrCfgDesc.o -c ./cfg/MatchSvrCfgDesc.c
	$(MAKE) -f makefile.real

clean:
	rm -f ./cfg/*.h ./cfg/*.c ./cfg/*.o ./cfg/*.tdr
	$(MAKE) -f makefile.real clean

rebuild: clean all

