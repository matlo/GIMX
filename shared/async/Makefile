include ../../Makedefs

LIBNAME=$(shell basename $(shell pwd)).a
ifneq ($(OS),Windows_NT)
OBJECTS := $(patsubst %.c,%.o,$(wildcard src/linux/*.c))
else
OBJECTS := $(patsubst %.c,%.o,$(wildcard src/windows/*.c))
endif
CPPFLAGS+=-Iinclude -Isrc -I../event/include

all: $(LIBNAME)

$(LIBNAME): $(OBJECTS)
	$(AR) $(ARFLAGS) $@ $?

clean:
	$(RM) $(LIBNAME) $(OBJECTS)
