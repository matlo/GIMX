include ../Makedefs

ifneq ($(OS),Windows_NT)
prefix=$(DESTDIR)/usr
bindir=$(prefix)/bin
endif

CPPFLAGS += -I../shared/configeditor/include -I../shared/controller/include
CFLAGS += -Wno-unused-local-typedefs `wx-config --cflags` -Winvalid-pch -include wx_pch.h -DWX_PRECOMP
LDFLAGS := ../shared/configeditor/configeditor.a \
      $(LDFLAGS) \
      ../shared/updater/updater.a \
      ../shared/configupdater/configupdater.a \
      `wx-config --libs` \
      `curl-config --libs` \
      -lxml2
ifeq ($(OS),Windows_NT)
LDFLAGS += -lSDL2 -lintl -liconv
endif

DEP = ../shared/configeditor/configeditor.a ../shared/event/GE.a ../shared/updater/updater.a ../shared/configupdater/configupdater.a
OBJ = launcherApp.o launcherMain.o
ifneq ($(OS),Windows_NT)
OUT = gimx-launcher
BINS = $(OUT)
else
OBJ += launcher.rc.o
OUT = gimx-launcher.exe
endif

all: $(OUT)

$(OUT): $(OBJ) $(DEP)
	$(LD) -o $(OUT) $(OBJ) $(LDFLAGS)

ifeq ($(OS),Windows_NT)
launcher.rc.o: launcher.rc
	WINDRES $^ -o $@
endif

%.o: %.cpp
	$(CPP) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJ) $(OUT)

.PHONY: clean

ifneq ($(OS),Windows_NT)
install: all
	mkdir -p $(prefix)
	mkdir -p $(bindir)
	mkdir -p $(prefix)/share/applications
	mkdir -p $(prefix)/share/pixmaps
	for i in $(BINS); do cp $$i $(bindir)/; done
	cp gimx-launcher.desktop $(prefix)/share/applications
	cp gimx-launcher.png $(prefix)/share/pixmaps

uninstall:
	-for i in $(BINS); do rm $(bindir)/$$i; done
	-rmdir $(bindir)
	-rmdir $(prefix)
	-rm $(prefix)/share/applications/gimx-launcher.desktop
	-rm $(prefix)/share/pixmaps/gimx-launcher.png

really-clean: clean uninstall
endif
