-include ../../../Makedefs

OBJS=common.o
LDLIBS=../GE.a ../../async/async.a
BINS=GE_test GE_test_hid
ifeq ($(OS),Windows_NT)
LDLIBS += $(shell sdl2-config --libs) -liconv -lws2_32 -lwinmm -lhid -lsetupapi
LDLIBS:=$(filter-out -mwindows,$(LDLIBS))
else
LDLIBS += -lXi -lX11
endif

CPPFLAGS +=-I../include -I../../async/include
DEPS=../GE.a ../../async/async.a

all: $(BINS)

clean:
	$(RM) $(BINS) *~ *.o

GE_test: $(OBJS) $(DEPS)

GE_test_hid: $(OBJS) $(DEPS)
