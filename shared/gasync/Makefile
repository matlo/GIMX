DIRS = src/common src/hid src/input src/poll src/prio src/serial src/timer src/usb

ifneq ($(OS),Windows_NT)
UHID=1
endif

ifeq ($(UHID),1)
DIRS := src/uhid $(DIRS)
endif

BUILDDIRS = $(DIRS:%=build-%)
INSTALLDIRS = $(DIRS:%=install-%)
CLEANDIRS = $(DIRS:%=clean-%)
UNINSTALLDIRS = $(DIRS:%=uninstall-%)

all: $(BUILDDIRS)
$(DIRS): $(BUILDDIRS)
$(BUILDDIRS):
	$(MAKE) -C $(@:build-%=%) UHID=$(UHID)

build-src/poll: build-src/common
build-src/input: build-src/hid build-src/poll
build-src/hid: build-src/common
build-src/serial: build-src/common
ifeq ($(OS),Windows_NT)
build-src/usb: build-src/timer build-src/poll
endif

clean: $(CLEANDIRS)
$(CLEANDIRS): 
	$(MAKE) -C $(@:clean-%=%) clean

install: $(INSTALLDIRS) all
$(INSTALLDIRS):
	$(MAKE) -C $(@:install-%=%) install

uninstall: $(UNINSTALLDIRS)
$(UNINSTALLDIRS):
	$(MAKE) -C $(@:uninstall-%=%) uninstall

.PHONY: subdirs $(DIRS)
.PHONY: subdirs $(BUILDDIRS)
.PHONY: subdirs $(CLEANDIRS)
.PHONY: all clean
