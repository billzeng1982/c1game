include ../../../global.mk

SOURCE = $(wildcard *.cpp) $(wildcard *.c)
OBJS = $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCE)))

OUTPUT = TestFileData TestFunction TestBsr

%.o:%.cpp
	@$(CXX) $(CXXFLAGS) -c $< $(INC_COMM) -I$(PATH_COMM)  -I./ -I../
	@echo [CXX] $@

.PHONY: all clean

all: $(OUTPUT)

TestFunction : TestFunction.o
	@$(CXX) $(CXXFLAGS) -o $@ $^
	@echo Build......$@

TestFileData : TestFileData.o
	@$(CXX) $(CXXFLAGS) -o $@ $^ $(LIB_COMM)
	@echo Build......$@

TestBsr : TestBsr.o
	@$(CXX) $(CXXFLAGS) -o $@ $^ $(LIB_COMM)
	@echo Build......$@
	
clean:
	rm -f $(OBJS)
	rm -f $(OUTPUT)

