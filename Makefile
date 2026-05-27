# Root Makefile — delegates to the active project folder
.PHONY: all clean dump

all:
	$(MAKE) -C blink

clean:
	$(MAKE) -C blink clean

dump:
	$(MAKE) -C blink dump


