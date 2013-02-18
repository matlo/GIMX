include ../Makedefs

prefix=$(DESTDIR)/usr
bindir=$(prefix)/bin

INC = -I../shared/configeditor/include
CFLAGS += `wx-config --cflags` -Winvalid-pch -include wx_pch.h -DWX_PRECOMP
LIBDIR =
LIB =../shared/configeditor/configeditor.a ../shared/event/GE.a -lxml2 `wx-config --libs`

DEP = ../shared/configeditor/configeditor.a ../shared/event/GE.a ../shared/updater/updater.o
OBJ = fpsconfigMain.o fpsconfigApp.o
OUT = gimx-fpsconfig

BINS = $(OUT)

all: $(OUT)

$(OUT): $(OBJ) $(DEP)
	$(LD) $(LDFLAGS) $(LIBDIR) -o $(OUT) $(OBJ) ../shared/updater/updater.o $(LIB)

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
	cp gimx-fpsconfig.desktop $(prefix)/share/applications
	cp gimx-fpsconfig.png $(prefix)/share/pixmaps

uninstall:
	-for i in $(BINS); do rm $(bindir)/$$i; done
	-rmdir $(bindir)
	-rmdir $(prefix)
	-rm $(prefix)/share/applications/gimx-fpsconfig.desktop
	-rm $(prefix)/share/pixmaps/gimx-fpsconfig.png

really-clean: clean uninstall
