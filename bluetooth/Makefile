include ../Makedefs

prefix=$(DESTDIR)/usr
bindir=$(prefix)/bin

BINS=gimx-bluetooth

INC = -I../shared/configeditor/include -I../shared/controller/include
CFLAGS += -Wno-unused-local-typedefs `wx-config --cflags` -Winvalid-pch -include wx_pch.h -DWX_PRECOMP
LIBDIR =
LIB = ../shared/configeditor/configeditor.a ../shared/event/GE.a -lxml2 `wx-config --libs` -Wl,--whole-archive ../shared/controller/controller.a -Wl,--no-whole-archive

DEP = ../shared/configeditor/configeditor.a ../shared/controller/controller.a ../shared/event/GE.a ../shared/updater/updater.o ../shared/configupdater/configupdater.o
OBJ = bluetoothApp.o bluetoothMain.o
OUT = gimx-bluetooth

all: $(OUT)

$(OUT): $(OBJ) $(DEP)
	$(LD) $(LDFLAGS) $(LIBDIR) -o $(OUT) $(OBJ) ../shared/updater/updater.o ../shared/configupdater/configupdater.o $(LIB)

%.o: %.cpp
	$(CPP) $(CFLAGS) $(INC) -c -o $@ $<

clean:
	rm -f $(OBJ) $(OUT)

.PHONY: clean

install: all
	mkdir -p $(prefix)
	mkdir -p $(bindir)
	mkdir -p $(prefix)/share/applications
	mkdir -p $(prefix)/share/pixmaps
	for i in $(BINS); do cp $$i $(bindir)/; done
	cp gimx-bluetooth.desktop $(prefix)/share/applications
	cp gimx-bluetooth.png $(prefix)/share/pixmaps

uninstall:
	-for i in $(BINS); do rm $(bindir)/$$i; done
	-rmdir $(bindir)
	-rmdir $(prefix)
	-rm $(prefix)/share/applications/gimx-bluetooth.desktop
	-rm $(prefix)/share/pixmaps/gimx-bluetooth.png

really-clean: clean uninstall
