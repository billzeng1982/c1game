include ../../../global.mk

SOURCE = $(wildcard *.cpp) $(wildcard *.c)
OBJS = $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCE)))

OUTPUT = TestReconnectDB

%.o:%.cpp
	@$(CXX) $(CXXFLAGS) -c $< $(INC_COMM) $(INC_TSF4G) $(INC_MYSQL) -I./ -I../ 
	@echo [CXX] $@

#LDENV = LD_RUN_PATH=$(shell /usr/bin/mysql_config --variable=pkglibdir)

.PHONY: all clean

all: $(OUTPUT)

TestReconnectDB : TestReconnectDB.o
	@$(LDENV) $(CXX) $(CXXFLAGS) -o $@ $^ $(LIB_COMM) $(LIB_MYSQL)
	@echo Build......$@

clean:
	rm -f $(OBJS)
	rm -f $(OUTPUT)
