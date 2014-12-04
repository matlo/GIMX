include ../../Makedefs

LIBNAME=$(shell basename $(shell pwd)).a
OBJECTS := $(patsubst %.c,%.o,$(wildcard src/*.c))
CPPFLAGS+=-Iinclude -Isrc

all: $(LIBNAME)

$(LIBNAME): $(OBJECTS)
	$(AR) $(ARFLAGS) $@ $?

clean:
	$(RM) $(LIBNAME) $(OBJECTS)
