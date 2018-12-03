include ../../../global.mk

SOURCE = $(wildcard *.cpp) $(wildcard *.c)
OBJS = $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCE)))

OUTPUT = TestLog_r

%.o:%.cpp
	@$(CXX) $(CXXFLAGS) -c $< -I$(INC_COMM) $(INC_TSF4G) -I./ -I../ -I../../
	@echo [CXX] $@

.PHONY: all clean

all: $(OUTPUT)

TestLog_r : TestLog_r.o
	@$(CXX) $(CXXFLAGS) -o $@ $^ $(LIB_COMM) $(LIB_TSF4G) -lpthread
	@echo Build......$@

clean:
	rm -f $(OBJS)
	rm -f $(OUTPUT)


