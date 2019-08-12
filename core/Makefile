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
          $(GIMXUSB_LDLIBS)

LDFLAGS += $(GIMXFILE_LDFLAGS) \
           $(GIMXHID_LDFLAGS) \
           $(GIMXGPP_LDFLAGS) \
           $(GIMXCONTROLLER_LDFLAGS) \
           $(GIMXINPUT_LDFLAGS) \
           $(GIMXPOLL_LDFLAGS) \
           $(GIMXPRIO_LDFLAGS) \
           $(GIMXSERIAL_LDFLAGS) \
           $(GIMXTIMER_LDFLAGS) \
           $(GIMXUSB_LDFLAGS)

CPPFLAGS += `xml2-config --cflags`
LDLIBS += `xml2-config --libs`

ifneq ($(OS),Windows_NT)
LDLIBS += -lm `pkg-config --libs ncursesw` -lbluetooth -lmhash
else
LDLIBS += $(shell sdl2-config --libs) -lws2_32 -lpdcursesw -lintl
LDLIBS:=$(filter-out -mwindows,$(LDLIBS))
endif
CPPFLAGS += -Iinclude -Iconnectors/btstack/include -Iconnectors/btstack/src

OBJECTS := $(patsubst %.c,%.o,$(wildcard *.c))
OBJECTS += $(patsubst %.c,%.o,$(wildcard connectors/btstack/*.c))
OBJECTS += $(patsubst %.c,%.o,$(wildcard connectors/btstack/src/*.c))
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
else
OBJECTS += $(patsubst %.c,%.o,$(wildcard system/windows/*.c))
OBJECTS += $(patsubst %.c,%.o,$(wildcard connectors/windows/*.c))
OBJECTS += $(patsubst %.c,%.o,$(wildcard connectors/windows/*/*.c))
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