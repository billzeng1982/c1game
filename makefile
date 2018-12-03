.PHONY: all clean 

all:
	$(MAKE) -C common all
	$(MAKE) -C services
	
clean:
	$(MAKE) -C common clean
	$(MAKE) -C services clean

