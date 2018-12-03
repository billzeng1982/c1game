include ../../global.mk

.PHONY: all clean rebuild

all:
	$(TDR) -H --add_custom_prefix="m_" ./cfg/RoleSvrCfgDesc.xml -O ./cfg  
	$(TDR) -C ./cfg/RoleSvrCfgDesc.xml -o ./cfg/RoleSvrCfgDesc.c 
	$(CC) $(CFLAGS) -o ./cfg/RoleSvrCfgDesc.o -c ./cfg/RoleSvrCfgDesc.c
	$(MAKE) -f makefile.real

clean:
	rm -f ./cfg/*.h ./cfg/*.c ./cfg/*.o
	$(MAKE) -f makefile.real clean

rebuild: clean all

