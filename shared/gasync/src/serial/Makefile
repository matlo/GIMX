ifeq ($(OS),Windows_NT)
OBJECTS += $(patsubst %.c,%.o,$(wildcard windows/*.c))
OBJECTS += $(patsubst %.c,%.o,$(wildcard ../common/windows/*.c))
else
OBJECTS += $(patsubst %.c,%.o,$(wildcard posix/*.c))
OBJECTS += $(patsubst %.c,%.o,$(wildcard ../common/posix/*.c))
endif

CPPFLAGS += -I../../include -I../
CFLAGS += -fPIC

include ../../../Makedefs
