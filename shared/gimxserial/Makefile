ifeq ($(OS),Windows_NT)
OBJECTS += $(patsubst %.c,%.o,$(wildcard src/windows/*.c))
OBJECTS += $(patsubst %.c,%.o,$(wildcard ../common/src/windows/*.c))
else
OBJECTS += $(patsubst %.c,%.o,$(wildcard src/posix/*.c))
OBJECTS += $(patsubst %.c,%.o,$(wildcard ../common/src/posix/*.c))
endif

CPPFLAGS += -Iinclude -I../
CFLAGS += -fPIC

include ../Makedefs
