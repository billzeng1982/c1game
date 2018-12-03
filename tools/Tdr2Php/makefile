include ../../global.mk

TARGET = Tdr2Php

TINYXML_DIR = $(PATH_COMM)/tinyxml2

SOURCE =  $(wildcard *.cpp)
SOURCE += $(wildcard $(TINYXML_DIR)/*.cpp)

OBJS = $(patsubst %.cpp,%.o,$(SOURCE))

./%.o : ./%.cpp
	@$(CXX) $(CXXFLAGS) -c -o $@ $< -I./ -I$(TINYXML_DIR) $(INC_TSF4G) -I$(PATH_COMM)/algo
	@echo Compile ...... $@

$(TINYXML_DIR)/%.o : $(TINYXML_DIR)/%.cpp
	@$(CXX) $(CXXFLAGS) -c -o $@ $<
	@echo Compile ...... $@

.PHONY : all clean

all : $(TARGET)

$(TARGET) : $(OBJS)
	@$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)
	@echo Build ...... $@

clean:
	@rm -f $(OBJS)
	@rm -f $(TARGET)
	@echo Clean ......
	