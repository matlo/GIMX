include ../Makedefs

ifneq ($(OS),Windows_NT)
prefix=$(DESTDIR)/usr
bindir=$(prefix)/bin
endif

CPPFLAGS += -I../shared/configeditor/include -I../shared/controller/include
CPPFLAGS += `wx-config --cflags` -Winvalid-pch -include wx_pch.h -DWX_PRECOMP `curl-config --cflags`
CFLAGS += -Wno-unused-local-typedefs
LDFLAGS := ../shared/configeditor/configeditor.a \
      $(LDFLAGS) \
      ../shared/updater/updater.a \
      `wx-config --libs` \
      `curl-config --libs` \
      -lxml2
ifeq ($(OS),Windows_NT)
LDFLAGS += -lSDL2 -lintl -liconv
endif

DEP = ../shared/configeditor/configeditor.a ../shared/controller/controller.a ../shared/event/GE.a ../shared/updater/updater.a
OBJ = configMain.o configApp.o
ifneq ($(OS),Windows_NT)
OUT = gimx-config
BINS = $(OUT)
else
OBJ += config.rc.o
OUT = gimx-config.exe
endif

all: $(OUT)

$(OUT): $(OBJ) $(DEP)
	$(LD) -o $(OUT) $(OBJ) $(LDFLAGS)

ifeq ($(OS),Windows_NT)
config.rc.o: config.rc
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
	cp gimx-config.desktop $(prefix)/share/applications
	cp gimx-config.png $(prefix)/share/pixmaps

uninstall:
	-for i in $(BINS); do rm $(bindir)/$$i; done
	-rmdir $(bindir)
	-rmdir $(prefix)
	-rm $(prefix)/share/applications/gimx-config.desktop
	-rm $(prefix)/share/pixmaps/gimx-config.png

really-clean: clean uninstall
endif
