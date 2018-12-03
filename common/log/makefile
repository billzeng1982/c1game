include ../../global.mk

SOURCE = $(wildcard *.cpp) $(wildcard *.c)

OBJ_FILES = $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCE)))

%.o:%.cpp
	@$(CXX) $(CXXFLAGS) -c $< $(INC_TSF4G) -I./ -I../ -I../utils -I../thread
	@echo [CXX] $@

%.o:%.c
	@$(CC) $(CFLAGS) -c $< $(INC_TSF4G) -I./ -I../ -I../utils
	@echo [CC] $@

all:$(OBJ_FILES)
	$(AR) q ../$(COMM_LIB_FILE) $(OBJ_FILES)

.PHONY: clean

clean:
	rm -f $(OBJ_FILES)

