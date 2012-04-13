CPP = g++
LD = g++
AR = ar

INC =
CFLAGS = -Wall -O2 `wx-config --cflags` -Winvalid-pch -include wx_pch.h -DWX_PRECOMP
RESINC = 
RCFLAGS = 
LIBDIR =
LIB = `wx-config --libs` -lpthread
LDFLAGS = -s

DEP = 
OBJ = sixstatusApp.o sixstatusMain.o
OUT = gimx-status

all: $(OUT)

$(OUT): $(OBJ) $(DEP)
	$(LD) $(LDFLAGS) $(LIBDIR) -o $(OUT) $(OBJ) $(LIB)

%.o: %.cpp
	$(CPP) $(CFLAGS) $(INC) -c -o $@ $<

clean:
	rm -f $(OBJ) $(OUT)

.PHONY: clean

install: all
	mkdir -p $(prefix)
	mkdir -p $(bindir)
	for i in $(BINS); do cp $$i $(bindir)/; done

uninstall:
	-for i in $(BINS); do rm $(bindir)/$$i; done
	-rmdir $(bindir)
	-rmdir $(prefix)

really-clean: clean uninstall
