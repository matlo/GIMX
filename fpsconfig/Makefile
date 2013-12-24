include ../Makedefs

ifneq ($(OS),Windows_NT)
prefix=$(DESTDIR)/usr
bindir=$(prefix)/bin
endif

INC = -I../shared/configeditor/include
CFLAGS += -Wno-unused-local-typedefs `wx-config --cflags` -Winvalid-pch -include wx_pch.h -DWX_PRECOMP
LIBDIR =
LIB =../shared/configeditor/configeditor.a ../shared/event/GE.a `wx-config --libs`
ifneq ($(OS),Windows_NT)
LIB += -lxml2
else 
LIB += -lSDL -llibxml2 -lintl -liconv
endif

DEP = ../shared/configeditor/configeditor.a ../shared/event/GE.a ../shared/updater/updater.o
OBJ = fpsconfigMain.o fpsconfigApp.o
ifneq ($(OS),Windows_NT)
OUT = gimx-fpsconfig
BINS = $(OUT)
else
OBJ += fpsconfig.rc.o
OUT = gimx-fpsconfig.exe
endif

all: $(OUT)

$(OUT): $(OBJ) $(DEP)
	$(LD) $(LDFLAGS) $(LIBDIR) -o $(OUT) $(OBJ) ../shared/updater/updater.o $(LIB)

ifeq ($(OS),Windows_NT)
fpsconfig.rc.o: fpsconfig.rc
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
	cp gimx-fpsconfig.desktop $(prefix)/share/applications
	cp gimx-fpsconfig.png $(prefix)/share/pixmaps

uninstall:
	-for i in $(BINS); do rm $(bindir)/$$i; done
	-rmdir $(bindir)
	-rmdir $(prefix)
	-rm $(prefix)/share/applications/gimx-fpsconfig.desktop
	-rm $(prefix)/share/pixmaps/gimx-fpsconfig.png

really-clean: clean uninstall
endif
