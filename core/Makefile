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

CPPFLAGS += $(GIMXINPUT_CPPFLAGS)

LDLIBS += $(GIMXFILE_LDLIBS) \
          $(GIMXHID_LDLIBS) \
          $(GIMXGPP_LDLIBS) \
          $(GIMXCONTROLLER_LDLIBS) \
          $(GIMXINPUT_LDLIBS) \
          $(GIMXPOLL_LDLIBS) \
          $(GIMXPRIO_LDLIBS) \
          $(GIMXSERIAL_LDLIBS) \
          $(GIMXTIMER_LDLIBS) \
          $(GIMXUSB_LDLIBS) \
          $(GIMXUDP_LDLIBS)

LDFLAGS += $(GIMXFILE_LDFLAGS) \
           $(GIMXHID_LDFLAGS) \
           $(GIMXGPP_LDFLAGS) \
           $(GIMXCONTROLLER_LDFLAGS) \
           $(GIMXINPUT_LDFLAGS) \
           $(GIMXPOLL_LDFLAGS) \
           $(GIMXPRIO_LDFLAGS) \
           $(GIMXSERIAL_LDFLAGS) \
           $(GIMXTIMER_LDFLAGS) \
           $(GIMXUSB_LDFLAGS) \
           $(GIMXUDP_LDFLAGS)

CPPFLAGS += `xml2-config --cflags`
LDLIBS += `xml2-config --libs`

ifneq ($(PDCURSES),1)
ifneq ($(OS),Windows_NT)
CPPFLAGS +=  `ncursesw5-config --cflags`
LDLIBS += `ncursesw5-config --libs`
else
CPPFLAGS +=  `ncursesw6-config --cflags`
LDLIBS += `ncursesw6-config --libs`
endif
else
CFLAGS += -DPDCURSES
LDLIBS += -lpdcursesw
endif

ifneq ($(OS),Windows_NT)
LDLIBS += -lm -lbluetooth -lmhash
else
LDLIBS += $(shell sdl2-config --libs) -lws2_32 -lintl
LDLIBS:=$(filter-out -mwindows,$(LDLIBS))
endif
CPPFLAGS += -I. -Iconnectors/bluetooth/btstack/include -Iconnectors/bluetooth/btstack/src

OBJECTS := $(patsubst %.c,%.o,$(wildcard *.c))
OBJECTS += $(patsubst %.c,%.o,$(wildcard connectors/bluetooth/*.c))
OBJECTS += $(patsubst %.c,%.o,$(wildcard connectors/bluetooth/btstack/*.c))
OBJECTS += $(patsubst %.c,%.o,$(wildcard connectors/bluetooth/btstack/src/*.c))
OBJECTS += $(patsubst %.c,%.o,$(wildcard connectors/*.c))
OBJECTS += $(patsubst %.c,%.o,$(wildcard connectors/gpp/*.c))
OBJECTS += $(patsubst %.c,%.o,$(wildcard connectors/report2event/*.c))
OBJECTS += $(patsubst %.c,%.o,$(wildcard haptic/*.c))
OBJECTS += $(patsubst %.c,%.o,$(wildcard haptic/source/*.c))
OBJECTS += $(patsubst %.c,%.o,$(wildcard haptic/sink/*.c))
OBJECTS += $(patsubst %.c,%.o,$(wildcard haptic/common/*.c))
ifneq ($(OS),Windows_NT)
OBJECTS += $(patsubst %.c,%.o,$(wildcard system/linux/*.c))
OBJECTS += $(patsubst %.c,%.o,$(wildcard connectors/linux/*/*.c))
OBJECTS += $(patsubst %.c,%.o,$(wildcard connectors/bluetooth/linux/*.c))
else
OBJECTS += $(patsubst %.c,%.o,$(wildcard system/windows/*.c))
OBJECTS += $(patsubst %.c,%.o,$(wildcard connectors/windows/*.c))
OBJECTS += $(patsubst %.c,%.o,$(wildcard connectors/windows/*/*.c))
OBJECTS += $(patsubst %.c,%.o,$(wildcard connectors/bluetooth/windows/*.c))
#OBJECTS:=$(filter-out connectors/sixaxis.o,$(OBJECTS))
OBJECTS:=$(filter-out connectors/btds4.o,$(OBJECTS))
endif

all: $(BINS)

gimx: $(OBJECTS)

clean:
	$(RM) $(OBJECTS) $(BINS)

ifneq ($(OS),Windows_NT)
install: all
	mkdir -p $(prefix)
	mkdir -p $(bindir)
	for i in $(BINS); do cp $$i $(bindir)/; done
	for i in $(BINS); do chmod ug+s $(bindir)/$$i; done

uninstall:
	-for i in $(BINS); do $(RM) $(bindir)/$$i; done
	-rmdir $(bindir)
	-rmdir $(prefix)

really-clean: clean uninstall
endif