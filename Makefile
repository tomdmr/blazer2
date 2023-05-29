PIO         = ~/.platformio/penv/bin/pio
ESPOTA      = python /home/tom/.platformio/packages/framework-arduinoespressif32/tools/espota.py
MYIP        = 192.168.13.13
HASH        = ~/c/djb2

JSC_SRC     = $(wildcard www/*.js)
CSS_SRC     = $(wildcard www/*.css)
HTM_SRC     = $(wildcard www/*.html)
ALL_WWW_SRC = $(JSC_SRC) $(CSS_SRC) $(HTM_SRC)
CSS_HASH  = $(foreach name, $(CSS_SRC), data/$(shell $(HASH) /$(patsubst www/%,%,$(name))).css)
JSC_HASH  = $(foreach name, $(JSC_SRC), data/$(shell $(HASH) /$(patsubst www/%,%,$(name))).js)
HTM_HASH  = $(foreach name, $(HTM_SRC), data/$(shell $(HASH) /$(patsubst www/%,%,$(name))).html)
ALL_WWW_HASH = $(CSS_HASH) $(JSC_HASH) $(HTM_HASH)


all: .depend.mak .depend.mak.done $(ALL_WWW_HASH) 

.depend.mak:	$(ALL_WWW_SRC)
	bash ./mkdepend

-include .depend.mak

upload:
	$(PIO) run --target=upload
upload_ota:
	$(PIO) run --target=upload --upload-port=$(TARGETIP)

uploadfs: all
	$(PIO) run --target=uploadfs

uploadfs_ota: all
	$(PIO) run --target=uploadfs --upload-port=$(TARGETIP)

debug:
	@echo "Sources: " $(ALL_WWW_SRC)
	@echo "Targets: " $(ALL_WWW_HASH)
