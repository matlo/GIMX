-include ../../../Makedefs

OBJS=common.o GE_test.o
LDLIBS=../GE.a
BINS=GE_test
ifeq ($(OS),Windows_NT)
LDLIBS += $(shell sdl2-config --libs) -liconv -lws2_32 -lwinmm
LDLIBS:=$(filter-out -mwindows,$(LDLIBS))
else
LDLIBS += -lXi -lX11
endif

CPPFLAGS +=-I../include
DEPS=../GE.a

all: $(BINS)

clean:
	$(RM) $(BINS) *~ *.o

GE_test: $(OBJS) $(DEPS)
