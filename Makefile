DIRS = utils shared core config launcher fpsconfig

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

build-core: build-shared
build-config: build-shared
build-launcher: build-shared
build-fpsconfig: build-shared

clean: $(CLEANDIRS)
$(CLEANDIRS): 
	$(MAKE) -C $(@:clean-%=%) clean

ifeq ($(OS),Windows_NT)
install: all
	mkdir -p setup
ifeq ($(MSYSTEM),MINGW32)
	cp -u -f /$(MSYSTEM)/bin/libgcc_s_dw2-1.dll setup
else
	cp -u -f /$(MSYSTEM)/bin/libgcc_s_seh-1.dll setup
endif
	cp -u -f /$(MSYSTEM)/bin/libiconv-2.dll setup
	cp -u -f /$(MSYSTEM)/bin/libintl-8.dll setup
	cp -u -f /$(MSYSTEM)/bin/libpdcursesw.dll setup
	cp -u -f /$(MSYSTEM)/bin/SDL2.dll setup
	cp -u -f /$(MSYSTEM)/bin/libusb-1.0.dll setup
	cp -u -f /$(MSYSTEM)/bin/libxml2-2.dll setup
	cp -u -f /$(MSYSTEM)/bin/liblzma-5.dll setup
	cp -u -f /$(MSYSTEM)/bin/libwinpthread-1.dll setup
	cp -u -f /$(MSYSTEM)/bin/libstdc++-6.dll setup
	cp -u -f /$(MSYSTEM)/bin/zlib1.dll setup
	cp -u -f /$(MSYSTEM)/bin/libcurl-4.dll setup
	cp -u -f /$(MSYSTEM)/bin/LIBEAY32.dll setup
	cp -u -f /$(MSYSTEM)/bin/libidn-11.dll setup
	cp -u -f /$(MSYSTEM)/bin/librtmp-1.dll setup
	cp -u -f /$(MSYSTEM)/bin/libgmp-10.dll setup
	cp -u -f /$(MSYSTEM)/bin/libgnutls-30.dll setup
	cp -u -f /$(MSYSTEM)/bin/libhogweed-4-0.dll setup
	cp -u -f /$(MSYSTEM)/bin/libnettle-6-0.dll setup
	cp -u -f /$(MSYSTEM)/bin/libp11-kit-0.dll setup
	cp -u -f /$(MSYSTEM)/bin/libffi-6.dll setup
	cp -u -f /$(MSYSTEM)/bin/libtasn1-6.dll setup
	cp -u -f /$(MSYSTEM)/bin/libssh2-1.dll setup
	cp -u -f /$(MSYSTEM)/bin/SSLEAY32.dll setup
	cp -u -f /$(MSYSTEM)/bin/libhidapi-0.dll setup
	cp -u -f core/gimx setup/gimx.exe
	cp -u -f config/gimx-config setup/gimx-config.exe
	cp -u -f launcher/gimx-launcher setup/gimx-launcher.exe
	cp -u -f fpsconfig/gimx-fpsconfig setup/gimx-fpsconfig.exe
	mkdir -p setup/share/locale
	for translation in po/*.po; \
  do \
    po=$$(basename $${translation} .po); \
    mkdir -p setup/share/locale/$$po/LC_MESSAGES; \
    msgfmt -o setup/share/locale/$$po/LC_MESSAGES/gimx.mo po/$$po.po; \
  done
	mkdir -p setup/ssl/certs
	cp -u -f /etc/pki/ca-trust/extracted/pem/tls-ca-bundle.pem setup/ssl/certs/ca-bundle.crt

.PHONY: subdirs $(DIRS)
.PHONY: subdirs $(BUILDDIRS)
.PHONY: subdirs $(CLEANDIRS)
.PHONY: all install clean
else

install: $(INSTALLDIRS) all
$(INSTALLDIRS):
	$(MAKE) -C $(@:install-%=%) install

uninstall: $(UNINSTALLDIRS) all
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
