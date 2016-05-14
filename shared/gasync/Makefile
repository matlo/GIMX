include ../../Makedefs

LIBNAME=$(shell basename $(shell pwd)).a
OBJECTS := $(patsubst %.c,%.o,$(wildcard src/*.c))
OBJECTS += $(patsubst %.c,%.o,$(wildcard src/*/*.c))
OBJECTS += $(patsubst %.c,%.o,$(wildcard src/input/hid/*.c))
ifneq ($(OS),Windows_NT)
OBJECTS += $(patsubst %.c,%.o,$(wildcard src/*/posix/*.c))
OBJECTS += $(patsubst %.c,%.o,$(wildcard src/*/libusb/*.c))
OBJECTS += $(patsubst %.c,%.o,$(wildcard src/*/linux/*.c))
else
OBJECTS += $(patsubst %.c,%.o,$(wildcard src/*/windows/*.c))
OBJECTS += $(patsubst %.c,%.o,$(wildcard src/*/sdl/*.c))
endif
ifneq ($(UHID),1)
OBJECTS:=$(filter-out src/uhid/linux/uhidasync.o,$(OBJECTS))
OBJECTS:=$(filter-out src/input/hid/logitechwheel.o,$(OBJECTS))
endif
CPPFLAGS += -Iinclude -Isrc

all: $(LIBNAME)

$(LIBNAME): $(OBJECTS)
	$(AR) $(ARFLAGS) $@ $?

clean:
	$(RM) $(LIBNAME) $(OBJECTS)
