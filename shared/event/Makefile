include ../../Makedefs

LIBNAME=GE.a
OBJECTS := $(patsubst %.c,%.o,$(wildcard src/*.c))
ifneq ($(OS),Windows_NT)
OBJECTS += $(patsubst %.c,%.o,$(wildcard src/linux/*.c))
else
OBJECTS += $(patsubst %.c,%.o,$(wildcard src/windows/*.c))
OBJECTS += $(patsubst %.c,%.o,$(wildcard src/windows/winmm/*.c))
endif
CPPFLAGS +=-I/usr/include/ -Iinclude -Isrc

all: $(LIBNAME)

$(LIBNAME): $(OBJECTS)
	$(AR) $(ARFLAGS) $@ $?

clean:
	$(RM) $(LIBNAME) $(OBJECTS)
