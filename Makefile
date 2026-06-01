# Root Makefile — delegates to individual project folders
.PHONY: all blink blinkRGB switch helloOled clean dump

all: blink blinkRGB switch helloOled

blink:
	$(MAKE) -C blink

blinkRGB:
	$(MAKE) -C blinkRGB

switch:
	$(MAKE) -C switch

helloOled:
	$(MAKE) -C helloOled

clean:
	$(MAKE) -C blink clean
	$(MAKE) -C blinkRGB clean
	$(MAKE) -C switch clean
	$(MAKE) -C helloOled clean

dump:
	$(MAKE) -C blink dump
	$(MAKE) -C blinkRGB dump
	$(MAKE) -C switch dump
	$(MAKE) -C helloOled dump
