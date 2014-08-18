include ../Makedefs

ifneq ($(OS),Windows_NT)
prefix=$(DESTDIR)/usr
bindir=$(prefix)/bin
endif

CPPFLAGS += -I../shared/configeditor/include -I../shared/controller/include
CPPFLAGS += -Wno-unused-local-typedefs `wx-config --cflags` -Winvalid-pch -include wx_pch.h -DWX_PRECOMP `curl-config --cflags`
LDLIBS := ../shared/configeditor/configeditor.a \
      $(LDLIBS) \
      ../shared/updater/updater.a \
      ../shared/configupdater/configupdater.a \
      `wx-config --libs` \
      `curl-config --libs` \
      -lxml2
ifeq ($(OS),Windows_NT)
LDLIBS += -lSDL2 -lintl -liconv
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

$(OUT): $(DEP) $(OBJ)
	$(LD) $^ $(LOADLIBES) $(LDLIBS) -o $@

ifeq ($(OS),Windows_NT)
launcher.rc.o: launcher.rc
	WINDRES $^ -o $@
endif

clean:
	$(RM) $(OBJ) $(OUT)

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
	-for i in $(BINS); do $(RM) $(bindir)/$$i; done
	-$(RM) $(bindir)
	-$(RM) $(prefix)
	-$(RM) $(prefix)/share/applications/gimx-launcher.desktop
	-$(RM) $(prefix)/share/pixmaps/gimx-launcher.png

really-clean: clean uninstall
endif
