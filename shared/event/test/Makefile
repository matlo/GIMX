include ../Makedefs
-include ../../../Makedefs

OBJ=common.o GE_test.o
LIB=../GE.a
BINS=GE_test
ifeq ($(OS),Windows_NT)
LIB += $(shell sdl2-config --libs) -liconv
LIB:=$(filter-out -mwindows,$(LIB))
endif

INC=-I../include
DEPS=../GE.a

all: $(BINS)

clean:
	rm -f $(BINS) *~ *.o

GE_test: $(OBJ) $(DEPS)
	$(CC) $(CFLAGS) $(INC) -o $@ $(OBJ) $(LIB)

%.o: %.c
	$(CC) $(CFLAGS) $(INC) -o $@ -c $<

