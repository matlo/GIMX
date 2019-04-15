include ../Makedefs

ifneq ($(OS),Windows_NT)
prefix=$(DESTDIR)/usr
bindir=$(prefix)/bin
sharedir=$(prefix)/share
endif

NAME=$(shell basename "$(shell pwd)")

CPPFLAGS += `wx-config --cflags` -Winvalid-pch -include wx_pch.h -DWX_PRECOMP -Wno-unused-local-typedefs
LDLIBS += `wx-config --libs` -lstdc++

OBJECTS := $(patsubst %.cpp,%.o,$(wildcard *.cpp))

OUT = gimx-$(NAME)

BINS = $(OUT)

all: $(OUT)

$(OUT): $(OBJECTS)

clean:
	$(RM) $(OBJECTS) $(OUT)

.PHONY: clean

ifneq ($(OS),Windows_NT)
install: all
	mkdir -p $(prefix)
	mkdir -p $(bindir)
	for i in $(BINS); do cp $$i $(bindir)/; done
	mkdir -p $(sharedir)/gimx/firmware
	for i in firmware/*; do cp $$i $(sharedir)/gimx/firmware; done

uninstall:
	-$(RM) $(sharedir)/gimx/firmware/*
	-rmdir $(sharedir)/gimx/firmware
	-rmdir $(sharedir)/gimx
	-rmdir $(sharedir)
	-for i in $(BINS); do $(RM) $(bindir)/$$i; done
	-rmdir $(bindir)
	-rmdir $(prefix)

really-clean: clean uninstall
endif
