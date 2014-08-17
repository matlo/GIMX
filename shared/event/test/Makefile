include ../Makedefs
-include ../../../Makedefs

OBJ=common.o GE_test.o
LDFLAGS=../GE.a
BINS=GE_test
ifeq ($(OS),Windows_NT)
LDFLAGS += $(shell sdl2-config --libs) -liconv -lws2_32 -lwinmm
LDFLAGS:=$(filter-out -mwindows,$(LDFLAGS))
else
LDFLAGS += -lXi -lX11
endif

CPPFLAGS+=-I../include
DEPS=../GE.a

all: $(BINS)

clean:
	$(RM) $(BINS) *~ *.o

GE_test: $(OBJ) $(DEPS)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LDFLAGS)

