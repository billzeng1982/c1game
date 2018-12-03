include ../../global.mk

SOURCE = $(wildcard *.cpp) $(wildcard *.c)

OBJ_FILES = $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCE)))

INCLUDES=$(INC_TSF4G) -I./ -I../ -I../algo

%.o:%.cpp
	@$(CXX) $(CXXFLAGS) -c $< $(INCLUDES)
	@echo [CXX] $@

%.o:%.c
	@$(CC) $(CFLAGS) -c $< $(INCLUDES)
	@echo [CC] $@

all:$(OBJ_FILES)
	$(AR) q ../$(COMM_LIB_FILE) $(OBJ_FILES)

.PHONY: clean

clean:
	@rm -f $(OBJ_FILES)
	@echo clean all *.o
	
