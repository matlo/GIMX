include ../Makedefs

prefix=$(DESTDIR)/usr
bindir=$(prefix)/bin

INC = -I../shared/configeditor/include
CFLAGS += `wx-config --cflags` -Winvalid-pch -include wx_pch.h -DWX_PRECOMP
LIBDIR =
LIB = ../shared/configeditor/configeditor.a ../shared/event/GE.a -lxml2 `wx-config --libs`
LDFLAGS = -s

DEP = ../shared/configeditor/configeditor.a ../shared/event/GE.a
OBJ = configMain.o configApp.o
OUT = gimx-config

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
	cp gimx-config.desktop $(prefix)/share/applications
	cp gimx-config.png $(prefix)/share/pixmaps

uninstall:
	-for i in $(BINS); do rm $(bindir)/$$i; done
	-rmdir $(bindir)
	-rmdir $(prefix)
	-rm $(prefix)/share/applications/gimx-config.desktop
	-rm $(prefix)/share/pixmaps/gimx-config.png

really-clean: clean uninstall
