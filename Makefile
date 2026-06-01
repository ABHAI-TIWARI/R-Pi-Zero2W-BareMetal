# Root Makefile — delegates to individual project folders
.PHONY: all blink blinkRGB clean dump

all: blink blinkRGB

blink:
	$(MAKE) -C blink

blinkRGB:
	$(MAKE) -C blinkRGB

clean:
	$(MAKE) -C blink clean
	$(MAKE) -C blinkRGB clean

dump:
	$(MAKE) -C blink dump
	$(MAKE) -C blinkRGB dump

