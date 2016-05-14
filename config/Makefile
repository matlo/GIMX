include ../Makedefs

ifneq ($(OS),Windows_NT)
prefix=$(DESTDIR)/usr
bindir=$(prefix)/bin
endif

NAME=$(shell basename $(shell pwd))

CPPFLAGS += `pkg-config libusb-1.0 --cflags` -I../shared/configeditor/include -I../shared/controller/include
CPPFLAGS += -Wno-unused-local-typedefs `wx-config --cflags` -Winvalid-pch -include wx_pch.h -DWX_PRECOMP `curl-config --cflags`
LDLIBS := ../shared/configeditor/configeditor.a \
      $(LDLIBS) \
      ../shared/updater/updater.a \
      `wx-config --libs` \
      `curl-config --libs` \
      `pkg-config libusb-1.0 --libs` \
      -lxml2 \
      -lstdc++ \
      -lm
ifeq ($(OS),Windows_NT)
LDLIBS += -lSDL2 -lintl -liconv
endif

DEPS = ../shared/configeditor/configeditor.a ../shared/controller/controller.a ../shared/gasync/gasync.a ../shared/updater/updater.a

OBJECTS := $(patsubst %.cpp,%.o,$(wildcard *.cpp))

OUT = gimx-$(NAME)

ifneq ($(OS),Windows_NT)
BINS = $(OUT)
else
OBJECTS += $(NAME).rc.o
endif

all: updated $(OUT)

$(OUT): $(OBJECTS)

#this rule triggers an update when $(DEPS) is updated
updated: $(DEPS)
	touch updated
	$(RM) $(OUT)

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
	-for i in $(BINS); do $(RM) $(bindir)/$$i; done
	-$(RM) $(bindir)
	-$(RM) $(prefix)
	-$(RM) $(prefix)/share/applications/gimx-$(NAME).desktop
	-$(RM) $(prefix)/share/pixmaps/gimx-$(NAME).png

really-clean: clean uninstall
endif
