include ../Makedefs

ifneq ($(OS),Windows_NT)
prefix=$(DESTDIR)/usr
bindir=$(prefix)/bin
endif

NAME=$(shell basename "$(shell pwd)")

CPPFLAGS += `wx-config --cflags` -Winvalid-pch -include wx_pch.h -DWX_PRECOMP -Wno-unused-local-typedefs
LDLIBS += `wx-config --libs` -lstdc++ -lm

LDLIBS += $(GIMXCONFIGEDITOR_LDLIBS) $(GIMXUPDATER_LDLIBS)

LDFLAGS += $(GIMXCONFIGEDITOR_LDFLAGS) $(GIMXUPDATER_LDFLAGS)

OBJECTS := $(patsubst %.cpp,%.o,$(wildcard *.cpp))

OUT = gimx-$(NAME)

ifneq ($(OS),Windows_NT)
BINS = $(OUT)
else
OBJECTS += $(NAME).rc.o
endif

all: $(OUT)

$(OUT): $(OBJECTS)

ifeq ($(OS),Windows_NT)
$(NAME).rc.o: $(NAME).rc
	WINDRES $^ -o $@
endif

clean:
	$(RM) $(OBJECTS) $(OUT)

.PHONY: clean

ifneq ($(OS),Windows_NT)
install: all
	mkdir -p $(prefix)
	mkdir -p $(bindir)
	mkdir -p $(prefix)/share/applications
	mkdir -p $(prefix)/share/pixmaps
	for i in $(BINS); do cp $$i $(bindir)/; done
	cp gimx-$(NAME).desktop $(prefix)/share/applications
	cp gimx-$(NAME).png $(prefix)/share/pixmaps

uninstall:
	-$(RM) $(prefix)/share/pixmaps/gimx-$(NAME).png
	-$(RM) $(prefix)/share/applications/gimx-$(NAME).desktop
	-for i in $(BINS); do $(RM) $(bindir)/$$i; done
	-rmdir $(prefix)/share/pixmaps
	-rmdir $(prefix)/share/applications
	-rmdir $(bindir)
	-rmdir $(prefix)

really-clean: clean uninstall
endif
