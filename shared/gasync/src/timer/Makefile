ifeq ($(OS),Windows_NT)
OBJECTS += $(patsubst %.c,%.o,$(wildcard windows/*.c))
OBJECTS += ../common/windows/gerror.o
else
OBJECTS += $(patsubst %.c,%.o,$(wildcard linux/*.c))
endif

CPPFLAGS += -I../../include -I../
CFLAGS += -fPIC

include ../../../Makedefs
