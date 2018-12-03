include ../../../global.mk

SOURCE = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp,%.o,$(SOURCE))

OUTPUT = TestTransSimple

%.o:%.cpp
	@$(CXX) $(CXXFLAGS) -c $< -I$(INC_COMM) $(INC_TSF4G) -I./ -I../ -I../../
	@echo [CXX] $@

.PHONY: all clean

all: $(OUTPUT)

TestTransSimple : TestTransSimple.o
	@$(CXX) $(CXXFLAGS) -o $@ $^ $(LIB_COMM) $(LIB_TSF4G)
	@echo Build......$@

clean:
	rm -f $(OBJS)
	rm -f $(OUTPUT)

