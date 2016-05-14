-include ../../../Makedefs

OUT=ginput_test ghid_test gserial_test gtimer_test
OBJS=common.o
LDLIBS=-Wl,--whole-archive ../gasync.a -Wl,--no-whole-archive -lm
BINS=ginput_test ghid_test gserial_test gtimer_test
ifeq ($(OS),Windows_NT)
LDLIBS += $(shell sdl2-config --libs) -liconv -lws2_32 -lwinmm -lhid -lsetupapi
LDLIBS:=$(filter-out -mwindows,$(LDLIBS))
else
BINS+=guhid_test
LDLIBS += -lXi -lX11
endif
LDLIBS += `pkg-config libusb-1.0 --libs`

ifeq ($(UHID),1)
OUT += guhid_test
BINS += guhid_test
endif

CPPFLAGS +=-I../include
DEPS=../gasync.a

all: updated $(BINS)

clean:
	$(RM) $(BINS) *~ *.o
	
#this rule triggers an update when $(DEPS) is updated
updated: $(DEPS)
	touch updated
	$(RM) $(OUT)

ginput_test: $(OBJS)

ghid_test: $(OBJS)

guhid_test: $(OBJS)

gserial_test: $(OBJS)

gtimer_test:
