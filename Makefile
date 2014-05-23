DIRS=shared core config serial fpsconfig

ifneq ($(OS),Windows_NT)
DIRS+= bluetooth po
endif

all:
	+for i in $(DIRS); do cd $$i; make all; cd ..; done

clean:
	+for i in $(DIRS); do cd $$i; make clean; cd ..; done

ifeq ($(OS),Windows_NT)
install: all
	mkdir -p setup
ifeq ($(OS),Windows_NT)
	cp -u -f $(BINDIR)/bin/libgcc_s_dw2-1.dll setup
else
	cp -u -f $(BINDIR)/bin/libgcc_s_seh-1.dll setup
endif
	cp -u -f $(BINDIR)/bin/libiconv-2.dll setup
	cp -u -f $(BINDIR)/bin/libintl-8.dll setup
	cp -u -f $(BINDIR)/bin/libpdcursesw.dll setup/pdcurses.dll
	cp -u -f $(BINDIR)/bin/SDL2.dll setup
	cp -u -f $(BINDIR)/bin/libusb-1.0.dll setup
	cp -u -f $(BINDIR)/bin/libxml2-2.dll setup
	cp -u -f $(BINDIR)/bin/liblzma-5.dll setup
	cp -u -f $(BINDIR)/bin/libwinpthread-1.dll setup
	cp -u -f $(BINDIR)/bin/libstdc++-6.dll setup
	cp -u -f $(BINDIR)/bin/zlib1.dll setup
	cp -u -f $(BINDIR)/bin/libcurl-4.dll setup
	cp -u -f $(BINDIR)/bin/LIBEAY32.dll setup
	cp -u -f $(BINDIR)/bin/libidn-11.dll setup
	cp -u -f $(BINDIR)/bin/librtmp-1.dll setup
	cp -u -f $(BINDIR)/bin/libgmp-10.dll setup
	cp -u -f $(BINDIR)/bin/libgnutls-28.dll setup
	cp -u -f $(BINDIR)/bin/libhogweed-2-5.dll setup
	cp -u -f $(BINDIR)/bin/libnettle-4-7.dll setup
	cp -u -f $(BINDIR)/bin/libp11-kit-0.dll setup
	cp -u -f $(BINDIR)/bin/libffi-6.dll setup
	cp -u -f $(BINDIR)/bin/libtasn1-6.dll setup
	cp -u -f $(BINDIR)/bin/libssh2-1.dll setup
	cp -u -f $(BINDIR)/bin/SSLEAY32.dll setup
	cp -u -f core/gimx.exe setup
	cp -u -f config/gimx-config.exe setup
	cp -u -f serial/gimx-serial.exe setup
	cp -u -f fpsconfig/gimx-fpsconfig.exe setup
	mkdir -p setup/share/locale
	for translation in po/*.po; \
  do \
    po=$$(basename $${translation} .po); \
    mkdir -p setup/share/locale/$$po/LC_MESSAGES; \
    msgfmt -o setup/share/locale/$$po/LC_MESSAGES/gimx.mo po/$$po.po; \
  done

else
install: all
	for i in $(DIRS); do cd $$i; make install; cd ..; done

uninstall:
	-for i in $(DIRS); do cd $$i; make uninstall; cd ..; done

really-clean: clean uninstall
endif
