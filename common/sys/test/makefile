include ../../../global.mk

SOURCE = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp,%.o,$(SOURCE))

OUTPUT = TestAsyncLock

%.o:%.cpp
	@$(CXX) $(CXXFLAGS) -c $< -I$(INC_COMM) $(INC_TSF4G) -I./ -I../ -I../../
	@echo [CXX] $@

.PHONY: all clean

all: $(OUTPUT)

TestAsyncLock : TestAsyncLock.o
	@$(CXX) $(CXXFLAGS) -o $@ $^ $(LIB_COMM) $(LIB_TSF4G)
	@echo Build......$@

clean:
	rm -f $(OBJS)
	rm -f $(OUTPUT)

