include ../../../global.mk

SOURCE = $(wildcard *.cpp) $(wildcard *.c)
OBJS = $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCE)))

OUTPUT = TestEvent


TARGET_INC=-I./ $(INC_COMM)
TARGET_LIB=$(LIB_COMM) $(LIB_TSF4G)


%.o:%.cpp
	@$(CXX) $(CXXFLAGS) -c $< -I$(PATH_COMM) $(TARGET_INC)
	@echo [CXX] $@

.PHONY: all clean

all: $(OUTPUT)

TestEvent : TestEvent.o
	@$(CXX) $(CXXFLAGS) -o $@ $^ $(TARGET_LIB)
	@echo Build......$@

clean:
	rm -f $(OBJS)


