include ../Makedefs

ifneq ($(OS),Windows_NT)
prefix=$(DESTDIR)/usr
bindir=$(prefix)/bin
endif

BINS=gimx
ifneq ($(OS),Windows_NT)
OUT=$(BINS)
else
OUT=gimx.exe
endif

ifneq ($(OS),Windows_NT)
LDLIBS += -lxml2 -lm -lncursesw -lusb-1.0 -lbluetooth -lmhash
else
LDLIBS += $(shell sdl2-config --libs) `xml2-config --libs` -lws2_32 -liconv -lhid -lsetupapi -lpdcursesw -lintl -lusb-1.0 -lwinmm
LDLIBS:=$(filter-out -mwindows,$(LDLIBS))
endif
CPPFLAGS+=-Iinclude -I../shared/event/include -I../shared/controller/include
ifneq ($(OS),Windows_NT)
CPPFLAGS += -I/usr/include/libxml2
else
CPPFLAGS += -Iconnectors/windows/bluetooth/include -Iconnectors/windows/bluetooth/src
endif
DEPS=../shared/event/GE.a ../shared/controller/controller.a

OBJECTS := $(patsubst %.c,%.o,$(wildcard *.c))
OBJECTS += $(patsubst %.c,%.o,$(wildcard connectors/*.c))
OBJECTS += $(patsubst %.c,%.o,$(wildcard connectors/gpp/*.c))
OBJECTS += $(patsubst %.c,%.o,$(wildcard connectors/report2event/*.c))
ifneq ($(OS),Windows_NT)
OBJECTS += $(patsubst %.c,%.o,$(wildcard system/linux/*.c))
OBJECTS += $(patsubst %.c,%.o,$(wildcard connectors/linux/*/*.c))
else
OBJECTS += $(patsubst %.c,%.o,$(wildcard system/windows/*.c))
OBJECTS += $(patsubst %.c,%.o,$(wildcard connectors/windows/*.c))
OBJECTS += $(patsubst %.c,%.o,$(wildcard connectors/windows/*/*.c))
OBJECTS += $(patsubst %.c,%.o,$(wildcard connectors/windows/bluetooth/src/hci_cmds.c))
OBJECTS += $(patsubst %.c,%.o,$(wildcard connectors/windows/bluetooth/src/utils.c))
OBJECTS += $(patsubst %.c,%.o,$(wildcard connectors/windows/bluetooth/src/sdp_util.c))
OBJECTS:=$(filter-out connectors/sixaxis.o,$(OBJECTS))
OBJECTS:=$(filter-out connectors/btds4.o,$(OBJECTS))
endif

all: updated $(BINS)

gimx: $(OBJECTS)

clean:
	$(RM) $(OBJECTS)

#this rule triggers an update when $(DEPS) is updated
updated: $(DEPS)
	touch updated
	$(RM) $(OUT)

ifneq ($(OS),Windows_NT)
install: all
	mkdir -p $(prefix)
	mkdir -p $(bindir)
	for i in $(BINS); do cp $$i $(bindir)/; done
	for i in $(BINS); do chmod ug+s $(bindir)/$$i; done

uninstall:
	-for i in $(BINS); do $(RM) $(bindir)/$$i; done
	-$(RM) $(bindir)
	-$(RM) $(prefix)

really-clean: clean uninstall
endif
