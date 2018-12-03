include ../../global.mk

.PHONY: all clean rebuild xml

all:
	$(TDR) -H --add_custom_prefix="m_" ./cfg/FriendSvrCfgDesc.xml -O ./cfg  
	$(TDR) -C ./cfg/FriendSvrCfgDesc.xml -o ./cfg/FriendSvrCfgDesc.c 
	$(CC) $(CFLAGS) -o ./cfg/FriendSvrCfgDesc.o -c ./cfg/FriendSvrCfgDesc.c
	$(MAKE) -f makefile.real

clean:
	rm -f ./cfg/*.h ./cfg/*.c ./cfg/*.o ./cfg/*.tdr
	$(MAKE) -f makefile.real clean

rebuild: clean all

xml:
	$(TDR) -H --add_custom_prefix="m_" ./cfg/FriendSvrCfgDesc.xml -O ./cfg  
	$(TDR) -C ./cfg/FriendSvrCfgDesc.xml -o ./cfg/FriendSvrCfgDesc.c 