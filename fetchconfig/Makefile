include ../Makedefs


ifneq ($(OS),Windows_NT)
prefix=$(DESTDIR)/usr
bindir=$(prefix)/bin
endif


NAME=$(shell basename "$(shell pwd)")


ifneq ($(OS),Windows_NT)
LDLIBS += -lstdc++ -lm `pkg-config --libs ncursesw`
else
LDLIBS += -lws2_32 -lstdc++ -lpdcursesw -lintl
#LDLIBS:=$(filter-out -mwindows,$(LDLIBS))
endif

CPPFLAGS += -Iinclude -std=c++11

LDFLAGS += $(GIMXINPUT_LDFLAGS)   $(GIMXUSB_LDFLAGS) $(GIMXPOLL_LDFLAGS)
LDLIBS  += $(GIMXINPUT_LDLIBS)    $(GIMXUSB_LDLIBS)  $(GIMXPOLL_LDLIBS)
LDLIBS  += $(GIMXUPDATER_LDLIBS)  $(GIMXCONFIGUPDATER_LDLIBS)
LDFLAGS += $(GIMXUPDATER_LDFLAGS) $(GIMXCONFIGUPDATER_LDFLAGS)


OBJECTS := $(patsubst %.cpp,%.o,$(wildcard *.cpp))

OUT=gimx-$(NAME)

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
	for i in $(BINS); do cp $$i $(bindir)/; done
	for i in $(BINS); do chmod ug+s $(bindir)/$$i; done

uninstall:
	-for i in $(BINS); do $(RM) $(bindir)/$$i; done
	-rmdir $(bindir)
	-rmdir $(prefix)

really-clean: clean uninstall
endif
