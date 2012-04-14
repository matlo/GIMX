include ../Makedefs

prefix=$(DESTDIR)/usr
bindir=$(prefix)/bin

INC = 
CFLAGS += `wx-config --cflags` -Winvalid-pch -include wx_pch.h -DWX_PRECOMP
LIBDIR =
LIB = `wx-config --libs`
LDFLAGS = -s 

DEP = 
OBJ = serialApp.o serialMain.o
OUT = gimx-serial

all: $(OUT)

$(OUT): $(OBJ) $(DEP)
	$(LD) $(LDFLAGS) $(LIBDIR) -o $(OUT) $(OBJ)  ../shared/updater/updater.o ../shared/configupdater/configupdater.o $(LIB)

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
	cp gimx-serial.desktop $(prefix)/share/applications
	cp gimx-serial.png $(prefix)/share/pixmaps

uninstall:
	-for i in $(BINS); do rm $(bindir)/$$i; done
	-rmdir $(bindir)
	-rmdir $(prefix)
	-rm $(prefix)/share/applications/gimx-serial.desktop
	-rm $(prefix)/share/pixmaps/gimx-serial.png

really-clean: clean uninstall
