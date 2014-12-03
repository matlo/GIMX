DIRS=shared core config launcher fpsconfig

ifneq ($(OS),Windows_NT)
DIRS+= po
endif

all:
	+for i in $(DIRS); do $(MAKE) -C $$i all; done

clean:
	+for i in $(DIRS); do $(MAKE) -C $$i clean; done

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
	cp -u -f /$(MSYSTEM)/bin/libgnutls-28.dll setup
	cp -u -f /$(MSYSTEM)/bin/libhogweed-2-5.dll setup
	cp -u -f /$(MSYSTEM)/bin/libnettle-4-7.dll setup
	cp -u -f /$(MSYSTEM)/bin/libp11-kit-0.dll setup
	cp -u -f /$(MSYSTEM)/bin/libffi-6.dll setup
	cp -u -f /$(MSYSTEM)/bin/libtasn1-6.dll setup
	cp -u -f /$(MSYSTEM)/bin/libssh2-1.dll setup
	cp -u -f /$(MSYSTEM)/bin/SSLEAY32.dll setup
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

else
install: all
	for i in $(DIRS); do $(MAKE) -C $$i install; done

uninstall:
	-for i in $(DIRS); do $(MAKE) -C $$i uninstall; done

really-clean: clean uninstall
endif
