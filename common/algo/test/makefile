include ../../../global.mk

SOURCE = $(wildcard *.cpp) $(wildcard *.c)
OBJS = $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCE)))

OUTPUT =  ExampleList test_list_sort TestHashCache TestDynArray TestPriorityQ TestBitMap TestFifo TestIndexedPQ TestIndexedPQ_2 TestSkipList TestRankSys TestStrSearch

%.o:%.cpp
	@$(CXX) $(CXXFLAGS) -c $< $(INC_COMM) $(INC_TSF4G) -I./ -I../ 
	@echo [CXX] $@

.PHONY: all clean

all: $(OUTPUT)

ExampleList : ExampleList.o
	@$(CXX) $(CXXFLAGS) -o $@ $^ $(LIB_COMM)
	@echo Build......$@

test_list_sort : test_list_sort.o
	@$(CXX) $(CXXFLAGS) -o $@ $^ $(LIB_COMM)
	@echo Build......$@

TestHashCache : TestHashCache.o
	@$(CXX) $(CXXFLAGS) -o $@ $^ $(LIB_COMM)
	@echo Build......$@

TestDynArray : TestDynArray.o
	@$(CXX) $(CXXFLAGS) -o $@ $^ $(LIB_COMM)
	@echo Build......$@

TestPriorityQ : TestPriorityQ.o
	@$(CXX) $(CXXFLAGS) -o $@ $^ $(LIB_COMM)
	@echo Build......$@

TestBitMap : TestBitMap.o
	@$(CXX) $(CXXFLAGS) -o $@ $^ $(LIB_COMM)
	@echo Build......$@

TestFifo : TestFifo.o
	@$(CXX) $(CXXFLAGS) -o $@ $^ $(LIB_COMM)
	@echo Build......$@

TestIndexedPQ : TestIndexedPQ.o
	@$(CXX) $(CXXFLAGS) -o $@ $^ $(LIB_COMM)
	@echo Build......$@

TestIndexedPQ_2 : TestIndexedPQ_2.o
	@$(CXX) $(CXXFLAGS) -o $@ $^ $(LIB_COMM)
	@echo Build......$@

TestSkipList : TestSkipList.o
	@$(CXX) $(CXXFLAGS) -o $@ $^ $(LIB_COMM)
	@echo Build......$@

TestRankSys : TestRankSys.o
	@$(CXX) $(CXXFLAGS) -o $@ $^ $(LIB_COMM)
	@echo Build......$@

TestStrSearch : TestStrSearch.o
	@$(CXX) $(CXXFLAGS) -o $@ $^ $(LIB_COMM)
	@echo Build......$@

clean:
	rm -f $(OBJS)
	rm -f $(OUTPUT)

