-include ../../../Makedefs

OBJS=common.o
LDLIBS=../gasync.a -lm
BINS=ginput_test ghid_test gserial_test gtimer_test
ifeq ($(OS),Windows_NT)
LDLIBS += $(shell sdl2-config --libs) -liconv -lws2_32 -lwinmm -lhid -lsetupapi
LDLIBS:=$(filter-out -mwindows,$(LDLIBS))
else
BINS+=guhid_test
LDLIBS += -lXi -lX11 -lusb-1.0
endif

CPPFLAGS +=-I../include
DEPS=../gasync.a

all: $(BINS)

clean:
	$(RM) $(BINS) *~ *.o

ginput_test: $(OBJS) $(DEPS)

ghid_test: $(OBJS) $(DEPS)

guhid_test: $(OBJS) $(DEPS)

gserial_test: $(OBJS) $(DEPS)

gtimer_test: $(DEPS)
