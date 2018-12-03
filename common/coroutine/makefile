include ../../global.mk

SOURCE = $(wildcard *.cpp)

OBJ_FILES = $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCE)))

TARGET_INC=-I./ $(INC_COMM)
TARGET_LIB=$(LIB_COMM) $(LIB_TSF4G)

%.o:%.cpp
	@$(CXX) $(CXXFLAGS) -c $< $(INC_TSF4G) $(TARGET_INC)
	@echo [CXX] $@

all:$(OBJ_FILES)
	$(AR) q ../$(COMM_LIB_FILE) $(OBJ_FILES)

.PHONY: clean test

TEST_DIR = test
test:
	$(MAKE) -C $(TEST_DIR) all

clean:
	rm -f $(OBJ_FILES)
	cd $(TEST_DIR) && $(MAKE) clean

