
include ../../global.mk

SOURCE = $(wildcard *.cpp) $(wildcard *.c)

OBJ_FILES = $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCE)))

%.o:%.cpp
	@$(CXX) $(CXXFLAGS) -c $< $(INC_TSF4G) -I./ -I../sys -I../mng -I../utils -I../
	@echo [CXX] $@

all:$(OBJ_FILES)
	$(AR) q ../$(COMM_LIB_FILE) $(OBJ_FILES)

.PHONY: clean test

TEST_DIR = test
test:
	$(MAKE) -C $(TEST_DIR) all

clean:
	rm -f $(OBJ_FILES)
	-cd $(TEST_DIR) && $(MAKE) clean

