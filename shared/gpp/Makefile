include ../../Makedefs

LIBNAME=$(shell basename $(shell pwd)).a
OBJECTS := $(patsubst %.c,%.o,$(wildcard *.c))
CPPFLAGS:=-I../gasync/include

all: $(LIBNAME)

$(LIBNAME): $(OBJECTS)
	$(AR) $(ARFLAGS) $@ $?

clean:
	$(RM) $(LIBNAME) $(OBJECTS)
