include ../../../global.mk

SOURCE = $(wildcard *.cpp) $(wildcard *.c)
OBJS = $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCE)))

OUTPUT = TestDynMempool TestMempool

%.o:%.cpp
	@$(CXX) $(CXXFLAGS) -c $< -I$(INC_COMM) $(INC_TSF4G) -I./ -I../ -I../../
	@echo [CXX] $@

.PHONY: all clean

all: $(OUTPUT)

TestDynMempool : TestDynMempool.o
	@$(CXX) $(CXXFLAGS) -o $@ $^ $(LIB_COMM)
	@echo Build......$@

TestMempool : TestMempool.o
	@$(CXX) $(CXXFLAGS) -o $@ $^ $(LIB_COMM)
	@echo Build......$@

clean:
	rm -f $(OBJS)
	rm -f $(OUTPUT)
