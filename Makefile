# Root Makefile — delegates to individual project folders
.PHONY: all blink blinkRGB switch helloOled oledBtnLed clean dump

all: blink blinkRGB switch helloOled oledBtnLed

blink:
	$(MAKE) -C blink

blinkRGB:
	$(MAKE) -C blinkRGB

switch:
	$(MAKE) -C switch

helloOled:
	$(MAKE) -C helloOled

oledBtnLed:
	$(MAKE) -C oledBtnLed

clean:
	$(MAKE) -C blink clean
	$(MAKE) -C blinkRGB clean
	$(MAKE) -C switch clean
	$(MAKE) -C helloOled clean
	$(MAKE) -C oledBtnLed clean

dump:
	$(MAKE) -C blink dump
	$(MAKE) -C blinkRGB dump
	$(MAKE) -C switch dump
	$(MAKE) -C helloOled dump
	$(MAKE) -C oledBtnLed dump
