include ../Makedefs

ifneq ($(OS),Windows_NT)
prefix=$(DESTDIR)/usr
bindir=$(prefix)/bin
endif

INC = -I../shared/configeditor/include -I../shared/controller/include
CFLAGS += -Wno-unused-local-typedefs `wx-config --cflags` -Winvalid-pch -include wx_pch.h -DWX_PRECOMP `curl-config --cflags`
LIBDIR =
LIB = ../shared/configeditor/configeditor.a \
      ../shared/event/GE.a \
      `wx-config --libs` \
      -Wl,--whole-archive ../shared/controller/controller.a -Wl,--no-whole-archive \
      `curl-config --libs` \
      -lxml2
ifeq ($(OS),Windows_NT)
LIB += -lSDL2 -lintl -liconv
endif

DEP = ../shared/configeditor/configeditor.a ../shared/controller/controller.a ../shared/event/GE.a ../shared/updater/updater.o
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
	$(LD) $(LDFLAGS) $(LIBDIR) -o $(OUT) $(OBJ) ../shared/updater/updater.o $(LIB)

ifeq ($(OS),Windows_NT)
config.rc.o: config.rc
	WINDRES $^ -o $@
endif

%.o: %.cpp
	$(CPP) $(CFLAGS) $(INC) -c -o $@ $<

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
