DIRS = shared utils core config launcher fpsconfig loader fetchconfig


ifneq ($(OS),Windows_NT)
DIRS+= po
endif

BUILDDIRS = $(DIRS:%=build-%)
INSTALLDIRS = $(DIRS:%=install-%)
CLEANDIRS = $(DIRS:%=clean-%)
UNINSTALLDIRS = $(DIRS:%=uninstall-%)

all: $(BUILDDIRS)
$(DIRS): $(BUILDDIRS)
$(BUILDDIRS):
	$(MAKE) -C $(@:build-%=%)

build-utils: build-shared
build-core: build-shared
build-config: build-shared
build-launcher: build-shared
build-fpsconfig: build-shared
build-fetchconfig: build-shared

clean: $(CLEANDIRS)
$(CLEANDIRS):
	$(MAKE) -C $(@:clean-%=%) clean

ifeq ($(OS),Windows_NT)
DLLS = $(shell ntldd -R {core,config,fpsconfig,launcher,fetchconfig}/*.exe | grep mingw | sed "s/.*=> //g" | cut -d' ' -f 1 | sed 's/\\/\\\\/g' | xargs cygpath -u | sort | uniq)\
	   $(shell ntldd -R shared/*/*.dll | grep mingw | sed "s/.*=> //g" | cut -d' ' -f 1 | sed 's/\\/\\\\/g' | xargs cygpath -u | sort | uniq)
install: all
	mkdir -p setup
	for DLL in $(DLLS); \
    do \
	  cp -u -f $$DLL setup; \
    done
	cp -u -f core/gimx setup/gimx.exe
	cp -u -f config/gimx-config setup/gimx-config.exe
	cp -u -f launcher/gimx-launcher setup/gimx-launcher.exe
	cp -u -f fetchconfig/gimx-fetchconfig setup/gimx-fetchconfig.exe
	cp -u -f fpsconfig/gimx-fpsconfig setup/gimx-fpsconfig.exe
	cp -u -f loader/gimx-loader setup/gimx-loader.exe
	cp -u -f shared/gimxinput/src/windows/gamecontrollerdb.txt setup
	mkdir -p setup/share/locale
	for translation in po/*.po; \
  do \
    po=$$(basename "$${translation}" .po); \
    mkdir -p setup/share/locale/$$po/LC_MESSAGES; \
    msgfmt -o setup/share/locale/$$po/LC_MESSAGES/gimx.mo po/$$po.po; \
  done
	mkdir -p setup/ssl/certs
	cp -u -f /etc/pki/ca-trust/extracted/pem/tls-ca-bundle.pem setup/ssl/certs/ca-bundle.crt
	cp -u -f shared/*/*.dll setup
	mkdir -p setup/firmware
	for fw in loader/firmware/*.hex; \
  do \
    cp -u -f $$fw setup/firmware; \
  done
ifeq ($(MSYSTEM),MINGW64)
	cp -u -f /mingw64/bin/avrdude.exe setup
	cp -u -f /mingw64/bin/avrdude.conf setup
else
	cp -u -f /mingw32/bin/avrdude.exe setup
	cp -u -f /mingw32/bin/avrdude.conf setup
endif

.PHONY: subdirs $(DIRS)
.PHONY: subdirs $(BUILDDIRS)
.PHONY: subdirs $(CLEANDIRS)
.PHONY: all install clean
else

install: $(INSTALLDIRS) all
$(INSTALLDIRS):
	$(MAKE) -C $(@:install-%=%) install

uninstall: $(UNINSTALLDIRS)
$(UNINSTALLDIRS):
	$(MAKE) -C $(@:uninstall-%=%) uninstall

really-clean: clean uninstall

.PHONY: subdirs $(DIRS)
.PHONY: subdirs $(BUILDDIRS)
.PHONY: subdirs $(INSTALLDIRS)
.PHONY: subdirs $(UNINSTALLDIRS)
.PHONY: subdirs $(CLEANDIRS)
.PHONY: all install uninstall clean
endif
