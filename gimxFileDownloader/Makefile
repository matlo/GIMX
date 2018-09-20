include ../Makedefs

ifneq ($(OS),Windows_NT)
prefix=$(DESTDIR)/usr
bindir=$(prefix)/bin
endif

BINS=gimxFileDownloader
ifneq ($(OS),Windows_NT)
OUT=$(BINS)
else
OUT=gimxFileDownloader.exe
endif

ifneq ($(OS),Windows_NT)
LDLIBS += -lm `pkg-config --libs ncursesw`
else
LDLIBS += -lws2_32 -lstdc++ -lpdcursesw -lintl
LDLIBS:=$(filter-out -mwindows,$(LDLIBS))
endif
CPPFLAGS += -Iinclude

OBJECTS := $(patsubst %.cpp,%.o,$(wildcard *.cpp))

LDLIBS += $(GIMXUPDATER_LDLIBS) $(GIMXCONFIGUPDATER_LDLIBS)

LDFLAGS += $(GIMXUPDATER_LDFLAGS) $(GIMXCONFIGUPDATER_LDFLAGS) 

all: $(BINS)

gimxFileDownloader: $(OBJECTS)

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