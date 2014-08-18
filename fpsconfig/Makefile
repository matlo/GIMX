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
      `wx-config --libs` \
      `curl-config --libs` \
      -lxml2
ifeq ($(OS),Windows_NT)
LDLIBS += -lSDL2 -lintl -liconv
endif

DEP = ../shared/configeditor/configeditor.a ../shared/controller/controller.a ../shared/event/GE.a ../shared/updater/updater.a
OBJ = fpsconfigMain.o fpsconfigApp.o
ifneq ($(OS),Windows_NT)
OUT = gimx-fpsconfig
BINS = $(OUT)
else
OBJ += fpsconfig.rc.o
OUT = gimx-fpsconfig.exe
endif

all: $(OUT)

$(OUT): $(DEP) $(OBJ)
	$(LD) $^ $(LOADLIBES) $(LDLIBS) -o $@

ifeq ($(OS),Windows_NT)
fpsconfig.rc.o: fpsconfig.rc
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
	cp gimx-fpsconfig.desktop $(prefix)/share/applications
	cp gimx-fpsconfig.png $(prefix)/share/pixmaps

uninstall:
	-for i in $(BINS); do $(RM) $(bindir)/$$i; done
	-$(RM) $(bindir)
	-$(RM) $(prefix)
	-$(RM) $(prefix)/share/applications/gimx-fpsconfig.desktop
	-$(RM) $(prefix)/share/pixmaps/gimx-fpsconfig.png

really-clean: clean uninstall
endif
