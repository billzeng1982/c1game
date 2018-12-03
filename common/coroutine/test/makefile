include ../../../global.mk

SOURCE = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp,%.o,$(SOURCE))

OUTPUT = CoBaseTest GetDataTrans_example

TARGET_INC=-I./ $(INC_COMM)
TARGET_LIB=$(LIB_COMM) $(LIB_TSF4G)

%.o:%.cpp
	@$(CXX) $(CXXFLAGS) -c $< -I$(PATH_COMM) $(TARGET_INC)
	@echo [CXX] $@

.PHONY: all clean

all: $(OUTPUT)

CoBaseTest : CoBaseTest.o
	@$(CXX) $(CXXFLAGS) -o $@ $^ $(TARGET_LIB)
	@echo Build......$@

GetDataTrans_example : GetDataTrans_example.o
	@$(CXX) $(CXXFLAGS) -o $@ $^ $(TARGET_LIB)
	@echo Build......$@

clean:
	rm -f $(OBJS) $(OUTPUT)

